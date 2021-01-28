#include "daq.h"
#include <QApplication>
#include <QDebug>
//#include "controller.h"
#include "logger.h"
#include <algorithm>
#include "signalmodel.h"
#include "Utility/userSettings.h"
#include <exception>

#ifdef WIN32
extern "C" {
#include "AxsunOCTCapture.h"
#include "AxsunOCTControl_LW_C.h"
}
#endif


#define NUM_OF_FRAME_BUFFERS FRAME_BUFFER_SIZE
#define ALLOCATED_OVERRUN_BUFFER_SIZE ( 256 * FFT_DATA_SIZE )  // Overrun buffer for Ocelot Mode

namespace{
int gFrameNumber = 0;
int gDaqCounter = 0;
size_t gBufferLength;
}

//static uint8_t gDaqBuffer[ 256 * FFT_DATA_SIZE ];

/*
 * Constructor
 */
DAQ::DAQ()
{
    isRunning    = false;
    lastImageIdx = 0;
    missedImgs   = 0;
    gFrameNumber = NUM_OF_FRAME_BUFFERS - 1;

    if( !startDaq() )
    {
        LOG1( "DAQ: Failed to start DAQ")
    }

    logDecimation();
}


void DAQ::logDecimation()
{
    userSettings &settings = userSettings::Instance();
    m_daqDecimation = settings.getDaqIndexDecimation();
    m_imageDecimation = settings.getImageIndexDecimation();
    m_daqLevel = settings.getDaqLogLevel();
}

void DAQ::logAxErrorVerbose(int line, AxErr axErrorCode, int count)
{
    char errorVerbose[512];
    axGetErrorString(axErrorCode, errorVerbose);
    if(count){
        LOG4(line, count, int(axErrorCode), errorVerbose)
    } else{
        LOG3(line, int(axErrorCode), errorVerbose)
    }
}

void DAQ::logRegisterValue(int line, int registerNumber)
{
    uint16_t regVal{0};
    AxErr retval = axGetFPGARegister(registerNumber,&regVal,0);
    if(retval == AxErr::NO_AxERROR){
        QString registerValue = QString::number(regVal,16);
        LOG2(registerNumber, registerValue)
    } else {
        logAxErrorVerbose(line, retval);
    }
}

void DAQ::NewImageArrived(new_image_callback_data_t data, void *user_ptr)
{
    static uint32_t sLastImage = 0;
    static uint32_t lastGoodDaq = 0;
    static uint32_t badCount = 0;
    const uint32_t bytes_allocated{MAX_ACQ_IMAGE_SIZE};
    bool isNewData{false};

    auto* daq = static_cast<DAQ*>(user_ptr);

    if(daq){
        uint32_t imaging, last_packet, last_frame, last_image, frames_since_sync;//, dropped_packets;

        AxErr success = axGetStatus(data.session, &imaging, &last_packet, &last_frame, &last_image, &(daq->m_droppedPackets), &frames_since_sync);
        if(success != AxErr::NO_AxERROR) {
            daq->logAxErrorVerbose(__LINE__, success);
            return;
        }

        if(imaging && (sLastImage != last_image)){

            OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

            sLastImage = last_image;

            if(bytes_allocated >= data.required_buffer_size){
                request_prefs_t prefs{ };
                prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
                prefs.which_window = 1;
                prefs.average_number = 1;
                prefs.crop_width_total = 0;
                image_info_t info{ };

                if(daq->m_daqDecimation && (daq->m_daqLevel >= 2))
                {
                    LOG4(daq->m_daqCount, badCount,last_image, daq->m_droppedPackets);
                }

                AxErr success = axRequestImage(data.session, data.image_number, prefs,
                                               bytes_allocated, axsunData->acqData, &info);
                if(success != AxErr::NO_AxERROR)
                {
                    daq->logAxErrorVerbose(__LINE__, success, daq->m_daqCount);
                    isNewData = false;
                }
                else
                {
                    LOG4(daq->m_daqCount, info.image_number, daq->m_droppedPackets, daq->m_badCountAcc);
                    isNewData = true;
                }
            }
            if(isNewData)
            {
                daq->updateSector(axsunData);
                gFrameNumber = ++daq->m_daqCount % NUM_OF_FRAME_BUFFERS;
                badCount = daq->m_daqCount - lastGoodDaq - 1;
                lastGoodDaq = daq->m_daqCount;
                daq->m_badCountAcc += badCount;
                LOG4(gFrameNumber, daq->m_daqCount, badCount, daq->m_badCountAcc);
            }
        }
    }
    return;
}

DAQ::~DAQ()
{
}

void DAQ::initDaq()
{
    AxErr retval;
    int loopCount = NUM_OF_FRAME_BUFFERS - 1;
    LOG2(loopCount, m_daqDecimation);

    frameTimer.start();
    fileTimer.start(); // start a timer to provide frame information for recording.

    retval = axRegisterNewImageCallback(session, NewImageArrived, this);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

    retval = axSetLaserEmission(1, 0);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

    retval = axImagingCntrlEthernet(-1,0);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

    setLaserDivider();

}

IDAQ *DAQ::getSignalSource()
{
    return this;
}

/*
 *  run
 */
void DAQ::run( void )
{
    if( !isRunning )
    {
        isRunning = true;
        while( isRunning )
        {
            sleep(1);
        }
    }
}

void DAQ::setSubsampling(int speed)
{
    if(speed < m_subsamplingThreshold){
        m_subsamplingFactor = 2;
        setLaserDivider();
    } else {
        m_subsamplingFactor = 1;
        setLaserDivider();
    }
}

bool DAQ::getData( new_image_callback_data_t data)
{
    bool isNewData{false};

    const uint32_t bytes_allocated{MAX_ACQ_IMAGE_SIZE};

    gFrameNumber = ++m_daqCount % NUM_OF_FRAME_BUFFERS;

    if(bytes_allocated >= data.required_buffer_size){
        request_prefs_t prefs{ };
        prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
        prefs.which_window = 1;
        prefs.average_number = 1;
//        prefs.downsample = int(m_subsamplingFactor == 2); do not enable

        /**
         * The total number of A-scans to be retrieved. Set to 0 to retrieve the full image.
         * If the value exceeds the remaining A-scans available following crop_width_offset,
         * the remaining available A-scans in the image will be retrieved/displayed.
*/
        prefs.crop_width_total = 0;
        image_info_t info{ };

        OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

        AxErr success = axRequestImage(data.session, data.image_number, prefs,
                                       bytes_allocated, axsunData->acqData, &info);
        if(success != AxErr::NO_AxERROR) {
            if(m_daqDecimation && (m_daqLevel >= 2)){
                logAxErrorVerbose(__LINE__, success, m_daqCount);
            }
        } else {
            if(m_daqDecimation && (m_daqCount % m_daqDecimation == 0)){
                LOG4(m_daqCount, info.image_number, m_droppedPackets, m_badCountAcc);
            }
            isNewData = true;
        }

        gBufferLength = info.width;

        // write in frame information for recording/playback
        axsunData->frameCount = data.image_number;
        axsunData->timeStamp = fileTimer.elapsed();;
    }

    return isNewData;
}


/*
 * startDaq
 *
 * Start the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::startDaq()
{
    AxErr success = AxErr::NO_AxERROR;

    try {

        success = axOpenAxsunOCTControl(true);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }

        success = axStartSession(&session, 4);    // Start Axsun engine session
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }

        success = axNetworkInterfaceOpen(1);
        if(success != AxErr::NO_AxERROR){
            char errorMsg[512];
            axGetErrorString(success, errorMsg);
            LOG2(int(success), errorMsg)
        }

        while(m_numberOfConnectedDevices != 2){
            m_numberOfConnectedDevices = axCountConnectedDevices();
            LOG1(m_numberOfConnectedDevices)

            msleep(500); //TO DO - handle failure max number of retries 60 sec
        }


        const int framesUntilForceTrig {35};
        /*
         * The number of frames for which the driver will wait for a Image_sync signal before timing out and entering Force Trigger mode.
         * Defaults to 24 frames at session creation.  Values outside the range of [2,100] will be automatically coerced into this range.
         * 35 * 256 = 8960 A lines
         */
        msleep(100);

        success = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        msleep(100);

        success = axSetTrigTimeout(session, framesUntilForceTrig * 2);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        msleep(100);

        logRegisterValue(__LINE__, 2);
        logRegisterValue(__LINE__, 5);
        logRegisterValue(__LINE__, 6);
        success = axSetImageSyncSource(AxEdgeSource::LVDS,16.6,0);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        msleep(100);
        logRegisterValue(__LINE__, 2);
        logRegisterValue(__LINE__, 5);
        logRegisterValue(__LINE__, 6);

        success = axGetMessage(session, axMessage );
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        LOG1(axMessage)
    }  catch (std::exception& e) {
       LOG1(e.what())
    } catch(...){
        LOG1(__FUNCTION__)
    }

    return success == AxErr::NO_AxERROR;
}

bool DAQ::shutdownDaq()
{
    AxErr success = AxErr::NO_AxERROR;

    success = axStopSession(session);    // Stop Axsun engine session
    if(success != AxErr::NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
    }
    LOG1(int(success));

    return success == AxErr::NO_AxERROR;
}

void DAQ::setLaserDivider()
{
    if(m_numberOfConnectedDevices == 2)
    {
        const int subsamplingFactor = m_subsamplingFactor;
        if( subsamplingFactor > 0  && subsamplingFactor <= 4 )
        {
            LOG1(subsamplingFactor)
            AxErr success = axSetSubsamplingFactor(subsamplingFactor,0);
            if(success != AxErr::NO_AxERROR){
                logAxErrorVerbose(__LINE__, success);
            }
            success = axGetMessage( session, axMessage );
            if(success != AxErr::NO_AxERROR){
                logAxErrorVerbose(__LINE__, success);
            }
        }
    }
}

void DAQ::setDisplay(float angle, int direction)
{
    LOG2(angle, direction);
}
