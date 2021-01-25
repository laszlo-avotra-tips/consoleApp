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
    lapCounter   = 0;   // Ocelot lap counter
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
    m_decimation = settings.getImageIndexDecimation();
}

void DAQ::logAxErrorVerbose(int line, AxErr axErrorCode)
{
    char errorVerbose[512];
    axGetErrorString(axErrorCode, errorVerbose);
    LOG3(line, int(axErrorCode), errorVerbose)
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

    auto* daq = static_cast<DAQ*>(user_ptr);

    if(daq){
        uint32_t imaging, last_packet, last_frame, last_image, dropped_packets, frames_since_sync;
        AxErr success = axGetStatus(data.session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
        if(success != AxErr::NO_AxERROR) {
            daq->logAxErrorVerbose(__LINE__, success);
            return;
        }

        if(imaging && sLastImage != last_image){
            sLastImage = last_image;
            if(daq->getData(data)){
                OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);
                SignalModel::instance()->setBufferLength(gBufferLength);
                daq->updateSector(axsunData);
            }
        }
    }
    return;
}

DAQ::~DAQ()
{
}

void DAQ::init()
{
    LOG1("init")
}

void DAQ::stop()
{
    isRunning = false;
}

void DAQ::pause()
{

}

void DAQ::resume()
{

}

QString DAQ::getDaqLevel()
{
    return "";
}

long DAQ::getRecordLength() const
{
    return 0;
}

bool DAQ::configure()
{
    LOG1("configure")
    return true;
}

void DAQ::enableAuxTriggerAsTriggerEnable(bool)
{

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
        frameTimer.start();
        fileTimer.start(); // start a timer to provide frame information for recording.

        AxErr retval;
        int loopCount = NUM_OF_FRAME_BUFFERS - 1;
        LOG2(loopCount, m_decimation)
        LOG1("***** Thread: DAQ::run()");

        LOG1(isRunning);
        {
            retval = axRegisterNewImageCallback(session, NewImageArrived, this);
            if(retval != AxErr::NO_AxERROR){
                char errorMsg[512];
                axGetErrorString(retval, errorMsg);
                LOG2(int(retval), errorMsg)
            }
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

//ax set laser emission
        setLaserDivider();
        while( isRunning )
        {
// get data and only procede if the image is new.
//            if( getData() )
//            {
//                gFrameNumber = ++loopCount % NUM_OF_FRAME_BUFFERS;
//                auto* sm =  SignalModel::instance();
//                OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
////                LOG2(gFrameNumber, axsunData)
//                sm->setBufferLength(gBufferLength);

//                emit updateSector(axsunData);
//            }
//            yieldCurrentThread();

            msleep(60);
        }
    }
    if(shutdownDaq()){
        qDebug() << "Thread: DAQ::run stop";
    } else {
        qDebug() << "Thread: DAQ::run failed to shut down";
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
    static int32_t counter = 0;

    request_prefs_t prefs{ };
    image_info_t info{ };
    gFrameNumber = ++counter % NUM_OF_FRAME_BUFFERS;
    OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

    const uint32_t bytes_allocated{MAX_ACQ_IMAGE_SIZE};
    if(bytes_allocated >= data.required_buffer_size){
        prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
        prefs.which_window = 0;
        AxErr success = axRequestImage(data.session, data.image_number, prefs, bytes_allocated, axsunData->acqData, &info);
        if(success != AxErr::NO_AxERROR) {
            logAxErrorVerbose(__LINE__, success);
        } else {
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
    LOG0

    return success == AxErr::NO_AxERROR;
}

/*
 * stopDaq
 *
 * Stop the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::shutdownDaq()
{
    qDebug() << "***** DAQ::shutdownDaq()";
    AxErr success = AxErr::NO_AxERROR;

    success = axStopSession(session);    // Stop Axsun engine session
    if(success != AxErr::NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
    }
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
