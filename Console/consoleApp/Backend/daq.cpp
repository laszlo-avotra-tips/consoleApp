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

    // Use the 'void * user_ptr' argument to send this callback a pointer to your preallocated
    // buffer if you are retrieving the image data rather than just direct-displaying it, as well as
    // any other user resources needed in this callback (log handles, etc).
    //
    // Calling AxsunOCTCapture functions other than axRequestImage(), axGetImageInfo(), & axGetStatus()
    // in this callback may result in undefined behavior, due to the nature of this callback's
    // execution in a concurrent thread with the application's other calls to the AxsunOCTCapture API.
    //
    // New Image Arrived events are stored in a FIFO queue within the AxsunOCTCapture library.
    // If the time spent in this callback exceeds the period of new images enqueued in the Main Image
    // Buffer, unprocessed callback events will stack up indefinitely.  You can monitor the backlog
    // of callback events by comparing the image number for the current callback 'data.image_number'
    // with the 'last_image' argument of axGetStatus().
    //
    // Force-triggered images will also invoke this callback, with data.image_number = 0.

    LOG1 (data.image_number);

    uint32_t imaging, last_packet, last_frame, last_image, dropped_packets, frames_since_sync;
    AxErr success = axGetStatus(data.session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
    if(success != AxErr::NO_AxERROR) {
        logAxErrorVerbose(__LINE__, success);
        return;
    }

    // axGetImageInfo() not necessary here, since required buffer size and image number
    // are already provided in the callback's data argument.  It is safe to call if other image info
    // is needed prior to calling axRequestImage().

    // convert user_ptr from void back into a std::vector<uint8_t>
    auto& image_vector = *(static_cast<std::vector<uint8_t>*>(user_ptr));
    auto bytes_allocated = image_vector.size();

    if(bytes_allocated >= data.required_buffer_size) {		// insure memory allocation large enough
//        auto prefs = request_prefs_t{ .request_mode = AxRequestMode::RETRIEVE_AND_DISPLAY, .which_window = 1 };
        request_prefs_t prefs{};
        prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
        prefs.which_window = 0;

        auto info = image_info_t{};
        auto retval = axRequestImage(data.session, data.image_number, prefs, bytes_allocated, image_vector.data(), &info);
        if (retval == AxErr::NO_AxERROR) {
            LOG1 (info.width);
            if (info.force_trig){
                LOG1("Force triggered mode.");
            }
            else{
                LOG2(last_image, data.image_number);
            }

            // sleep timer to simulate additional user tasks in callback
//			std::this_thread::sleep_for(10us);
        }
        else{
            int axRequestImageReported{static_cast<int>(retval)};
            LOG1(axRequestImageReported);
        }
    }
    else{
        int MemoryAllocationTooSmallForRetrievalOfImage{static_cast<int>(data.image_number)};
        LOG1(MemoryAllocationTooSmallForRetrievalOfImage);
    }
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
        LOG1("***** Thread: DAQ::run()")

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

//        uint16_t reg2Val{0};
//        retval = axGetFPGARegister(2,&reg2Val,0);
//        if(retval == AxErr::NO_AxERROR){
//            bool bit2 = reg2Val & 0x4;
//            LOG2(reg2Val, bit2)
//        }
//        uint16_t reg19Val{0};
//        retval = axGetFPGARegister(19,&reg19Val,0);
//        if(retval == AxErr::NO_AxERROR){
//            bool bit15 = reg19Val & 0x8000;
//            LOG2(reg19Val,bit15)
//        }

//ax set laser emission
        setLaserDivider();
        LOG1(isRunning)

        while( isRunning )
        {

            // get data and only procede if the image is new.
            if( getData() )
            {
                gFrameNumber = ++loopCount % NUM_OF_FRAME_BUFFERS;
                auto* sm =  SignalModel::instance();
                OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
//                LOG2(gFrameNumber, axsunData)
                sm->setBufferLength(gBufferLength);

                emit updateSector(axsunData);
            }
            yieldCurrentThread();
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

/*
 * getData
 */
bool DAQ::getData( )
{
    bool isNewData{false};
    static int sprevReturnedImageNumber = -1;

    uint32_t imaging, last_packet, last_frame, last_image, dropped_packets, frames_since_sync;  // for axGetStatus()
    request_prefs_t prefs{ };
    image_info_t info{ };
    static int32_t counter = 0;

    // get Main Image Buffer status
    AxErr success = axGetStatus(session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
    if(success != AxErr::NO_AxERROR) {
        logAxErrorVerbose(__LINE__, success);
        return false;
    }

    ++counter;
    OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);


    if(imaging){
        success = axGetImageInfo(session, 0, &info);
        if(success != AxErr::NO_AxERROR) {
//            logAxErrorVerbose(__LINE__, success);
        } else {
            const uint32_t output_buf_len{MAX_ACQ_IMAGE_SIZE};
            prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
            prefs.which_window = 0;
            success = axRequestImage(session, info.image_number, prefs, output_buf_len, axsunData->acqData, &info);
            int currentImageNumber = info.image_number;
//                LOG2(counter, info.image_number)
            if(currentImageNumber != sprevReturnedImageNumber){
                sprevReturnedImageNumber = currentImageNumber;

                isNewData = true;
                gBufferLength = info.width;

                // write in frame information for recording/playback
                axsunData->frameCount = gDaqCounter;
                axsunData->timeStamp = fileTimer.elapsed();;
                axsunData->milliseconds = 30;

                gDaqCounter++;
            }
        }
    }


    yieldCurrentThread();

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
//        success = axUSBInterfaceOpen(1);
//        if(success != AxErr::NO_AxERROR){
//            char errorMsg[512];
//            axGetErrorString(success, errorMsg);
//            LOG2(int(success), errorMsg)
//        }

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


//        success = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
//        if(success != AxErr::NO_AxERROR){
//            logAxErrorVerbose(__LINE__, success);
//        }
//        msleep(100);

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

void DAQ::sendToAdvacedView(const OCTFile::OctData_t &od, int frameNumber)
{
    uint8_t const * const src{od.acqData};
    uint8_t * const dst{od.advancedViewFftData};
    SignalModel::instance()->setAdvacedViewSourceFrameNumber(frameNumber);

    if(src && dst){
        const size_t bufferLength{FFTDataSize};
        memcpy(dst,src,bufferLength);
        emit notifyAcqData();
    } else {
        qDebug() << " src =" << src << ", dst=" << dst;
    }
}
