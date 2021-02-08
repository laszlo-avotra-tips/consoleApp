#include "daq.h"
#include <QApplication>
#include <QDebug>
#include <QTextStream>

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
    m_disableRendering = settings.getDisableRendering();
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

DAQ::~DAQ()
{
}

void DAQ::initDaq()
{
    AxErr retval;

    frameTimer.start();
    fileTimer.start(); // start a timer to provide frame information for recording.

    // NewImageArrived - callback_function A user-supplied function to be called.
    // Pass NULL to un-register a callback function.
    retval = axRegisterNewImageCallback(session, NewImageArrived, this);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

    // emission_state =1 enables laser emission, =0 disables laser emission.
    // which_laser The numeric index of the desired Laser.
    const uint32_t emission_state{1};
    const uint32_t which_laser{0};
    retval = axSetLaserEmission(emission_state, which_laser);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

    // number_of_images =0 for Imaging Off (idle), =-1 for Live Imaging (no record), or any positive
    // value between 1 and 32767 to request the desired number of images in a Burst Record operation.
    // which_DAQ The numeric index of the desired DAQ.
    const int16_t number_of_images{-1};
    const uint32_t which_DAQ{0};
    retval = axImagingCntrlEthernet(number_of_images, which_DAQ);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

    setSubSampling();

}

IDAQ *DAQ::getSignalSource()
{
    return this;
}

void DAQ::setSubsampling(int speed)
{
    LOG2(speed, m_subsamplingThreshold)
    if(speed < m_subsamplingThreshold){
        m_subsamplingFactor = 2;
        setSubSampling();
    } else {
        m_subsamplingFactor = 1;
        setSubSampling();
    }
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

        const float capacity_MB{50.0f};
        success = axStartSession(&session, capacity_MB);    // Start Axsun engine session
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }

        //interface_status =1 opens the interface or resets an existing open interface, =0 closes the interface.
        const uint32_t interface_status{1};
        success = axNetworkInterfaceOpen(interface_status);
        if(success != AxErr::NO_AxERROR){
            char errorMsg[512];
            axGetErrorString(success, errorMsg);
            LOG2(int(success), errorMsg)
        }

        //the two network interfaces 192.168.10.1 and 10.2
        while(m_numberOfConnectedDevices != 2){
            m_numberOfConnectedDevices = axCountConnectedDevices();
            LOG1(m_numberOfConnectedDevices)

            QThread::msleep(500); //TO DO - handle failure max number of retries 60 sec
        }

        const int framesUntilForceTrigDefault {24};
        /*
         * The number of frames for which the driver will wait for a Image_sync signal before timing out and entering Force Trigger mode.
         * Defaults to 24 frames at session creation.  Values outside the range of [2,100] will be automatically coerced into this range.
         * 24 gives us 6144 Alines.
         */
        QThread::msleep(100);

        success = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        QThread::msleep(100);

        //framesUntilForceTrig The number of frames for which the driver will wait for a Image_sync signal
        //before timing out and entering Force Trigger mode.  Defaults to 24 frames at session creation.
        //Values outside the range of [2,100] will be automatically coerced into this range.
        success = axSetTrigTimeout(session, framesUntilForceTrigDefault * 4);
        LOG2(int(success),framesUntilForceTrigDefault);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        QThread::msleep(100);

        logRegisterValue(__LINE__, 2);
        logRegisterValue(__LINE__, 5);
        logRegisterValue(__LINE__, 6);

        // frequency The Image_sync frequency (Hz); this parameter is optional and is ignored when source
        //is not INTERNAL.
        const float frequency{16.6};
        const uint32_t which_DAQ{0};
        success = axSetImageSyncSource(AxEdgeSource::LVDS, frequency, which_DAQ);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        QThread::msleep(100);
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

void DAQ::setSubSampling()
{
    if(m_numberOfConnectedDevices == 2)
    {
        const int subsamplingFactor = m_subsamplingFactor;
        if( subsamplingFactor > 0  && subsamplingFactor <= 4 )
        {
            LOG1(subsamplingFactor);

            // subsampling_factor Subsampling factor must be between 1 and 166.
            // which_DAQ The numeric index of the desired DAQ.
            const uint32_t which_DAQ{0};
            AxErr success = axSetSubsamplingFactor(subsamplingFactor, which_DAQ);
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

void DAQ::NewImageArrived(new_image_callback_data_t data, void* user_ptr)
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

    auto* daq = static_cast<DAQ*>(user_ptr);

    if(daq)
    {
        daq->getData(data);
    }
}

bool DAQ::getData(new_image_callback_data_t data)
{

    QString msg;
    QTextStream qs(&msg);

    auto currentTime = QTime::currentTime();
    QString timeString = currentTime.toString("hh:mm:ss.zzz");
    qs << timeString << "\t" <<data.image_number << "\t";

    uint32_t imaging, last_packet, last_frame, last_image, dropped_packets, frames_since_sync;
    auto success = axGetStatus(data.session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
    if(success ==  AxErr::NO_AxERROR){
        if(dropped_packets != m_lastDroppedPacketCount)
        {
            m_lastDroppedPacketCount = dropped_packets;
            LOG1(dropped_packets);
        }
    } else {
        LOG1(int(success));
    }

    // axGetImageInfo() not necessary here, since required buffer size and image number
    // are already provided in the callback's data argument.  It is safe to call if other image info
    // is needed prior to calling axRequestImage().

    auto* sm = SignalModel::instance();
    OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
    const uint32_t bytes_allocated{MAX_ACQ_IMAGE_SIZE};
    gFrameNumber = ++m_daqCount % NUM_OF_FRAME_BUFFERS;

    auto info = image_info_t{};

    if (bytes_allocated >= data.required_buffer_size) {		// insure memory allocation large enough
        auto prefs = request_prefs_t{ .request_mode = AxRequestMode::RETRIEVE_TO_CALLER, .which_window = 1 };
        auto retval = axRequestImage(data.session, data.image_number, prefs, bytes_allocated, axsunData->acqData, &info);
        if (retval == AxErr::NO_AxERROR) {
            qs << "Success: \tWidth: " << info.width;
            if (info.force_trig)
                qs << "\tForce triggered mode.";
            else
                qs << "\tBacklog: " << (last_image - data.image_number);

            // sleep timer to simulate additional user tasks in callback
           QThread::usleep(10);
        }
        else
            qs << "axRequestImage reported: " << int(retval);
    }
    else
        qs << "Memory allocation too small for retrieval of image " << data.image_number;

//    if(!data.image_number) //forced trigger logging
//        LOG1(msg);

    if(data.image_number && (data.image_number % 16 == 0))
        LOG1(msg);

    if(data.image_number && !(last_image - data.image_number)){
        axsunData->bufferLength = info.width;

        // write in frame information for recording/playback
        axsunData->frameCount = data.image_number;
        axsunData->timeStamp = fileTimer.elapsed();;
        sm->pushImageRenderingQueue(*axsunData);
    }

    return true;
}
