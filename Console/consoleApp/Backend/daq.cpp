#include "daq.h"
#include <QApplication>
#include <QDebug>
#include <QTextStream>

#include "logger.h"
#include <algorithm>
#include "signalmodel.h"
#include "Utility/userSettings.h"
#include "mainScreen.h"

#include <exception>

#ifdef WIN32
extern "C" {
#include "AxsunOCTCapture.h"
#include "AxsunOCTControl_LW_C.h"
}
#endif

/*
 * Constructor
 */
DAQ::DAQ(MainScreen *ms) : m_mainScreen(ms)
{
    initLogLevelAndDecimation();
}


void DAQ::initLogLevelAndDecimation()
{
    userSettings &settings = userSettings::Instance();
    m_daqDecimation = settings.getDaqIndexDecimation();
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

bool DAQ::setLaserEmissionState(uint32_t emission_state)
{
    bool success{false};
    // emission_state =1 enables laser emission, =0 disables laser emission.
    // which_laser The numeric index of the desired Laser.
    const uint32_t which_laser{0};
    AxErr retval = axSetLaserEmission(emission_state, which_laser);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG3(emission_state, int(retval), errorMsg)
    } else {
        success = true;
        LOG1(emission_state)
    }
    return success;
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

    int frameBufferCount = userSettings::Instance().getNumberOfDaqBuffers();
    m_bufferNumber = frameBufferCount ? (frameBufferCount - 1) : 0;

    imageFrameTimer.start(); // start a timer to provide frame information

    // NewImageArrived - callback_function A user-supplied function to be called.
    // Pass NULL to un-register a callback function.
    retval = axRegisterNewImageCallback(session, NewImageArrived, this);
    if(retval != AxErr::NO_AxERROR){
        char errorMsg[512];
        axGetErrorString(retval, errorMsg);
        LOG2(int(retval), errorMsg)
    }

//    LOG1(turnLaserOn());

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

    m_callbackTimer.start();
    m_dataTimer.start();
//    setSubSamplingFactor();

}

IDAQ *DAQ::getSignalSource()
{
    return this;
}

void DAQ::setSubsamplingAndForcedTrigger(int speed)
{
    //set sbsampling
    LOG2(speed, m_subsamplingThreshold)
    if(speed < m_subsamplingThreshold){
        m_subsamplingFactor = 2;
        setSubSamplingFactor();
    } else {
        m_subsamplingFactor = 1;
        setSubSamplingFactor();
    }

    //framesUntilForceTrig The number of frames for which the driver will wait for a Image_sync signal
    //before timing out and entering Force Trigger mode.  Defaults to 24 frames at session creation.
    //Values outside the range of [2,100] will be automatically coerced into this range.
    //    const ForceTriggerTimeoutTable m_forceTriggerTimeoutTable
    //    {//   rpm  | timeout
    //        { 2000 ,   12   },
    //        { 1000 ,   23   },
    //        { 600  ,   20   },
    //        { 800  ,   15   }
    //    };
    int framesUntilForceTrig = m_framesUntilForceTrigDefault;
    const auto& line = m_forceTriggerTimeoutTable.find(speed);
    if(line != m_forceTriggerTimeoutTable.end()){
        framesUntilForceTrig = line->second;
    }

    AxErr success = axSetTrigTimeout(session, framesUntilForceTrig);
    LOG3(int(success), speed, framesUntilForceTrig);
    if(success != AxErr::NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
    }
    QThread::msleep(100);
}

/*
 * startDaq
 *
 * Start the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::startDaq()
{
    AxErr success = AxErr::NO_AxERROR;

    m_numberOfConnectedDevices = 0;

    //shutdownDaq();

    try {

        success = axOpenAxsunOCTControl(true);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }

        const float capacity_MB{500.0f};
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
    int errorCount = 0;

    success = axStopSession(session);    // Stop Axsun engine session
    if(success != AxErr::NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
        ++errorCount;
    }
    LOG1(int(success == AxErr::NO_AxERROR));

    success = axCloseAxsunOCTControl();
    if(success != AxErr::NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
        ++errorCount;
    }
    LOG1(int(success == AxErr::NO_AxERROR));

    return errorCount == 0;
}

bool DAQ::turnLaserOn()
{
    bool success{false};
    // emission_state =1 enables laser emission, =0 disables laser emission.
    // which_laser The numeric index of the desired Laser.

    if(m_numberOfConnectedDevices == 2){
        const uint32_t emission_state{1};
        LOG1(emission_state)
        success = setLaserEmissionState(emission_state);
    }
    return success;
}

bool DAQ::turnLaserOff()
{
    bool success{false};
    // emission_state =1 enables laser emission, =0 disables laser emission.
    // which_laser The numeric index of the desired Laser.

    if(m_numberOfConnectedDevices == 2){
        const uint32_t emission_state{0};
        LOG1(emission_state)
        success = setLaserEmissionState(emission_state);
    }
    return success;
}

void DAQ::setSubSamplingFactor()
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

void DAQ::getData(new_image_callback_data_t data)
{

    QString msg;
    QTextStream qs(&msg);
    static uint32_t m_frameNumberGoodLast = 0;
    static uint32_t missedImageCountAcc = 0;
    float axsunErrorPercent{0.0f};

    //QCoreApplication::processEvents();

    ++m_callbackCount;
    m_callbackTime = m_callbackTimer.elapsed();
    m_callbackTimer.restart();

    QElapsedTimer getDataFunctionTimer;
    getDataFunctionTimer.start();
    auto currentTime = QTime::currentTime();
    QString timeString = currentTime.toString("hh:mm:ss.zzz");
//    qs << timeString;
    qs << "\t" <<data.image_number << "\t";

    uint32_t imaging, last_packet, last_frame, last_image, dropped_packets, frames_since_sync;
    auto success = axGetStatus(data.session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
    if(success ==  AxErr::NO_AxERROR){
        if(dropped_packets != m_lastDroppedPacketCount)
        {
            m_lastDroppedPacketCount = dropped_packets;
            qs << "\tdropped_packets=" << dropped_packets;//todo1
        }
    } else {
        qs << "\tsucces=" << int(success);
    }

    // axGetImageInfo() not necessary here, since required buffer size and image number
    // are already provided in the callback's data argument.  It is safe to call if other image info
    // is needed prior to calling axRequestImage().

    auto* sm = SignalModel::instance();

    OCTFile::OctData_t* axsun{nullptr};
    int frameBufferCount = userSettings::Instance().getNumberOfDaqBuffers();

    if(userSettings::Instance().getIsSimulation() && (frameBufferCount > 1))
    {
        axsun = sm->getOctData(1);
//        LOG1(axsun.acqData)
    }
    else
    {

        m_bufferNumber = m_callbackCount % frameBufferCount;
        axsun = sm->getOctData(m_bufferNumber);
//        LOG1(axsun.acqData)
    }

    const uint32_t bytes_allocated{MAX_ACQ_IMAGE_SIZE};

    auto info = image_info_t{};

    AxErr retval{AxErr::BUFFER_IS_EMPTY};
    if (bytes_allocated >= data.required_buffer_size) {		// insure memory allocation large enough
        auto prefs = request_prefs_t{ .request_mode = AxRequestMode::RETRIEVE_TO_CALLER, .which_window = 1 };
        retval = axRequestImage(data.session, data.image_number, prefs, bytes_allocated, axsun->acqData, &info);
        axsun->bufferLength = info.width;
        axsun->frameNumber = data.image_number;
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
            qs << "\taxRequestImage reported: " << int(retval);
    }
    else {
        qs << "Memory allocation too small for retrieval of image " << data.image_number;
    }

    int32_t missedImageCount = 0;

    const bool thisFrameIsGood =
            (retval == AxErr::NO_AxERROR) &&
            axsun &&
            data.image_number &&
            !(last_image - data.image_number) &&
            axsun->bufferLength &&
            (axsun->bufferLength != 256);

    if( thisFrameIsGood){
        ++m_frameGoodCount;
        ++m_imageNumber;
        m_frameNumberGoodLast = axsun->frameNumber;

//        sm->pushImageRenderingQueue(axsun);
//        sm->setBufferNumber(m_bufferNumber);
//        if(m_mainScreen){
//            m_mainScreen->presentData(axsun);
//        }

    } else {
        ++m_frameBadCount;

        missedImageCount = axsun->frameNumber - m_frameNumberGoodLast - 1;
        if(m_frameNumberGoodLast && (m_frameNumberGoodLast < axsun->frameNumber) && (missedImageCount > 0) ){
            missedImageCountAcc +=missedImageCount;
        }
    }

    //reset stats at 300
    if(m_callbackCount == 30){
       m_frameGoodCount = 30;
       m_frameNumberGoodLast = 29;
    }
    axsun->callbackCount = m_callbackCount;
    axsun->frameCountGood = m_frameGoodCount;
    axsun->frameCountBad = m_callbackCount - m_frameGoodCount;
    axsun->frameNumberGoodLast = m_frameNumberGoodLast;

    axsun->imageNumber = m_imageNumber;

    axsun->timeStamp = imageFrameTimer.elapsed();
    axsun->index = m_bufferNumber;

    if(thisFrameIsGood && m_mainScreen){
//        sm->setBufferNumber(m_bufferNumber);
        sm->pushImageRenderingQueue(axsun);
    //m_mainScreen->presentData(axsun);
//        m_mainScreen->updateImage();
    }

    if(thisFrameIsGood){
    }

    m_dataTime = m_dataTimer.elapsed();
    m_dataTimer.restart();

    if(data.image_number && m_daqDecimation && (data.image_number % m_daqDecimation == 0)){
        axsunErrorPercent = 100.0f * axsun->frameCountBad / axsun->callbackCount;
//        LOG4(missedImageCountAcc, axsun->frameNumber, m_frameNumberGoodLast, percent);
        LOG4(m_bufferNumber, axsun->acqData, msg, getDataFunctionTimer.elapsed());
        LOG4(axsun->callbackCount, axsun->frameNumber, axsun->frameCountGood, axsun->frameCountBad);
        LOG3(axsun->frameNumberGoodLast, axsun->imageNumber, axsunErrorPercent);
//        LOG2(m_callbackTime, m_dataTime);
    }
}
