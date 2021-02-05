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
int gDaqCounter = 0;
//size_t gBufferLength;
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

void DAQ::NewImageArrived1(new_image_callback_data_t data, void *user_ptr)
{
    static uint32_t missedImageCount = 0;
    static int count{0};

    auto* daq = static_cast<DAQ*>(user_ptr);

    if(daq)
    {
        uint32_t imaging, last_packet, last_frame, last_image, frames_since_sync;
        uint32_t& dropped_packets = (daq->m_droppedPackets);

        AxErr success = axGetStatus(data.session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
        if(success != AxErr::NO_AxERROR)
        {
            daq->logAxErrorVerbose(__LINE__, success);
            return;
        }

//        if(imaging && (sLastImage != last_image))
        if(!(last_image - data.image_number))
        {
            daq->m_missedImagesCountAccumulated += missedImageCount;
            if(daq->m_daqDecimation && (++count % daq->m_daqDecimation == 0)){
                LOG2(count, last_image);
            }

            if(daq->getData(data))
            {
                auto* sm = SignalModel::instance();
                OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
//                daq->updateSector(axsunData);
                if(!daq->m_disableRendering){
                    sm->pushImageRenderingQueue(*axsunData);
                }
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

bool DAQ::getData1( new_image_callback_data_t data)
{
    bool isNewData{false};

    const uint32_t bytes_allocated{MAX_ACQ_IMAGE_SIZE};

    gFrameNumber = ++m_daqCount % NUM_OF_FRAME_BUFFERS;

    if(bytes_allocated >= data.required_buffer_size){
        request_prefs_t prefs{ };
        prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
        prefs.which_window = 1;
        prefs.average_number = 1;
        prefs.crop_width_total = 0;
        image_info_t info{ };

        OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

        AxErr success = axRequestImage(data.session, data.image_number, prefs,
                                       bytes_allocated, axsunData->acqData, &info);
        if(success != AxErr::NO_AxERROR)
        {
            if(m_daqLevel){
                logAxErrorVerbose(__LINE__, success, m_daqCount);
            }
        }
        else
        {
            if(m_daqDecimation && m_daqLevel && (m_daqCount % m_daqDecimation == 0))
            {
                LOG3(m_daqCount, info.image_number, m_missedImagesCountAccumulated);
            }
            isNewData = true;
        }

        //gBufferLength = info.width;
        axsunData->bufferLength = info.width;

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

            QThread::msleep(500); //TO DO - handle failure max number of retries 60 sec
        }


        const int framesUntilForceTrig {35};
        /*
         * The number of frames for which the driver will wait for a Image_sync signal before timing out and entering Force Trigger mode.
         * Defaults to 24 frames at session creation.  Values outside the range of [2,100] will be automatically coerced into this range.
         * 35 * 256 = 8960 A lines
         */
        QThread::msleep(100);

        success = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        QThread::msleep(100);

        success = axSetTrigTimeout(session, framesUntilForceTrig * 2);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        QThread::msleep(100);

        logRegisterValue(__LINE__, 2);
        logRegisterValue(__LINE__, 5);
        logRegisterValue(__LINE__, 6);
        success = axSetImageSyncSource(AxEdgeSource::LVDS,16.6,0);
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
        if(dropped_packets)
        {
//            std::cout << __LINE__ << ". dropped_packets " << dropped_packets << std::endl;
            LOG1(dropped_packets);
        }
    } else {
//        std::cout << __LINE__  << ". " << int(success) << std::endl;
        LOG1(int(success));
    }

    // axGetImageInfo() not necessary here, since required buffer size and image number
    // are already provided in the callback's data argument.  It is safe to call if other image info
    // is needed prior to calling axRequestImage().

//    // convert user_ptr from void back into a std::vector<uint8_t>
//    auto& image_vector = *(static_cast<std::vector<uint8_t>*>(user_ptr));
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

//    LOG1(msg);
    if(!data.image_number)
        LOG1(msg);

    if(data.image_number && (data.image_number % 16 == 0))
        LOG1(msg);

    if(!(last_image - data.image_number)){
        axsunData->bufferLength = info.width;

        // write in frame information for recording/playback
        axsunData->frameCount = data.image_number;
        axsunData->timeStamp = fileTimer.elapsed();;
        sm->pushImageRenderingQueue(*axsunData);
    }

    return true;
}
