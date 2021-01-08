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
//#include "stdafx.h"     // Axsun includes
extern "C" {
#include "AxsunOCTCapture.h"
#include "AxsunOCTControl_LW_C.h"
}
#endif

#define PCIE_MODE 0         // PCIE or Ethernet mode for non-synthetic
#define USE_LVDS_TRIGGER 1  // Use LVDS trigger (or LVCMOS)

#define UINT16_MAX_VAL 65535
#define NUM_OF_FRAME_BUFFERS FRAME_BUFFER_SIZE
#define ALLOCATED_OVERRUN_BUFFER_SIZE ( 256 * FFT_DATA_SIZE )  // Overrun buffer for Ocelot Mode

namespace{
int gFrameNumber = 0;
int gDaqCounter = 0;
size_t gBufferLength;
}

// Define AxsunOCTCapture class to wrap axStartSession(), axStopSession(), and the session handle in RAII fashion.
// (This will convieniently call axStopSession() automatically in the destructor so that resources are cleaned up.)
class AxsunOCTCapture {
private:
    AOChandle session_{ nullptr };
public:
    AxsunOCTCapture(int32_t capacity) { if (auto retval = axStartSession(&session_, capacity); retval != AxErr::NO_AxERROR) throw retval; }  // C++17
    ~AxsunOCTCapture() { axStopSession(session_); }
    AOChandle operator()() { if (session_) return session_; else throw AxErr::CAPTURE_SESSION_NOT_SETUP; }
};

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
        fileTimer.start(); // start a timer to provide frame information for recording.

        int loopCount = NUM_OF_FRAME_BUFFERS - 1;
        LOG2(loopCount, m_decimation)
        LOG1("***** Thread: DAQ::run()");
        auto retval = axOpenAxsunOCTControl(true);

//        char message[512];

//        try{
//            if (auto retval = axSetupDisplay(session, 0, 0, 0, 1024, 512, 0); retval != AxErr::NO_AxERROR) throw retval;
//            if (auto retval = axAdjustBrightnessContrast(session, 1, 0, 1); retval != AxErr::NO_AxERROR) throw retval;
//        }     catch (const AxErr& e) {
//            axGetErrorString(e, message);
//            LOG1 (message);
//        }
//        catch (...) {
//            QString errorMessage("***** UNKNOWN ERROR. Program terminating.");
//            LOG1(errorMessage);
//        }

//        while(m_numberOfConnectedDevices != 2){
//            m_numberOfConnectedDevices = axCountConnectedDevices();
//            LOG1(m_numberOfConnectedDevices)

//            msleep(500);
//        }
        sleep(1);
        setLaserDivider();

        while( isRunning )
        {

            // get data and only procede if the image is new.
            if( getData() )
            {
                gFrameNumber = ++loopCount % NUM_OF_FRAME_BUFFERS;
                auto* sm =  SignalModel::instance();
                OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
                LOG2(gFrameNumber, axsunData)
                sm->setBufferLength(gBufferLength);

                emit updateSector(axsunData);
            }
            yieldCurrentThread();
            msleep(65); //loop timer 65 for approximately 16 fps update rate
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

bool DAQ::getData1( )
{
    static bool isLoggingInitialized{false};
    uint32_t required_buffer_size = 0;
    uint32_t returned_image_number = 0;
    static uint32_t sprevReturnedImageNumber = 0;
    static int32_t lostImageCount = 0;
    static uint32_t imageCount = 0;
    static int64_t axErrorCount = 0;
    float lostImagesInPercent = 0.0f;
    int32_t width = 0;
    int32_t height = 0;
    AxDataType data_type = AxDataType::U8;
    uint32_t returned_image = 0;
    uint8_t force_trig = 0;
    uint8_t trig_too_fast = 0;
    static std::map<AxErr,int> errorTable;
    static int64_t force_trigCount = 0;
    static int64_t trig_too_fastCount = 0;
    AxErr success{AxErr::NO_AxERROR};
    static AxErr sRetVal{AxErr::NO_AxERROR};
    bool isReturnedImageNumberChanged{false};
    static int requestImageErrorCount = 0;

    int64_t requestedImageNumber = -1;

    if(session != session0){
        LOG2(session, session0)
                return false;
    }


    try{
        success = axGetImageInfoAdv(session, requestedImageNumber,
                                    &returned_image_number, &height, &width, &data_type, &required_buffer_size, &force_trig, &trig_too_fast );
    }  catch (std::exception& e) {
       LOG1(e.what())
       LOG2(requestedImageNumber,returned_image_number)
       LOG2(height, width)
       LOG2(int(data_type), required_buffer_size)
       LOG2(force_trig, trig_too_fast)
    } catch(...){
        LOG1(__FUNCTION__)
    }

    isReturnedImageNumberChanged = (success == AxErr::NO_AxERROR) && (returned_image_number != sprevReturnedImageNumber);
    if(!isReturnedImageNumberChanged){
        return false;
    }
    sprevReturnedImageNumber = returned_image_number;

    bool isReturn = false;

    if(success != sRetVal){
        sRetVal = success;
        if(success != AxErr::NO_AxERROR){
            if(auto it = errorTable.find(success) != errorTable.end()){
                errorTable[success]++;
            } else {
                errorTable[success] = 1;
            }
            ++axErrorCount;
            isReturn = true;
        }
    }

    if(success != AxErr::NO_AxERROR){
        isReturn = true;
    }

    if(isReturnedImageNumberChanged){
        if( force_trig == 1){
            ++force_trigCount;
        }
        if(required_buffer_size >= MAX_ACQ_IMAGE_SIZE){
            QString errorMsg("required_buffer_size >= myBufferSize");
            LOG3(errorMsg,required_buffer_size, MAX_ACQ_IMAGE_SIZE);
            isReturn = true;
        }
    }

    if( force_trig == 1){
        isReturn = true;
    }

    if(isReturn){
        return false;
    }

    //initialize logging
    if(!isLoggingInitialized){
        lastImageIdx = returned_image_number - 1;
        force_trigCount = 0;
        axErrorCount = 0;
        isLoggingInitialized = true;
        LOG4(m_count, returned_image_number, lostImageCount, lostImagesInPercent)
        LOG4(axErrorCount,required_buffer_size,height, width)
        LOG2(force_trigCount, trig_too_fastCount)
    }

    if(isReturnedImageNumberChanged ){
        ++m_count;
        ++imageCount;

        if(m_decimation && (m_count % m_decimation == 0)){
            lostImagesInPercent =  100.0f * lostImageCount / imageCount;
            LOG4(m_count, returned_image_number, lostImageCount, lostImagesInPercent)
            LOG4(axErrorCount,required_buffer_size,height, width)
            LOG3(session, force_trigCount, trig_too_fastCount)
            if(errorTable.size() >= 1){
                for(auto it = errorTable.begin(); it != errorTable.end(); ++it ){
                    AxErr error = it->first;
                    int errorCode = int(error);
                    auto errorCount = it->second;
                    char errorMsg[512];
                    axGetErrorString(error, errorMsg);
                    LOG3(errorCode, errorCount, errorMsg)
                }
             }
        }
        if( returned_image_number > (lastImageIdx + 1) ){
           lostImageCount += returned_image_number - lastImageIdx - 1;
        }

        if( returned_image_number <= lastImageIdx )
        {
            LOG2(returned_image_number, lastImageIdx)
            return false;
        }

//        lastImageIdx = returned_image_number;
        image_info_t info{ };
        request_prefs_t prefs{ };

        // if no errors, configure the request preferences and then request the image for display
        prefs.request_mode = AxRequestMode::DISPLAY_ONLY;
        prefs.which_window = 1;

        OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

//        success = axRequestImage( session,
//                                   returned_image_number,
//                                   &returned_image,
//                                   &height,
//                                   &width,
//                                   &data_type,
//                                   axsunData->acqData,
//                                   MAX_ACQ_IMAGE_SIZE );

        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
            ++requestImageErrorCount;
            LOG3(returned_image_number, returned_image, requestImageErrorCount)
            return false;
        }
        lastImageIdx = returned_image;

        gBufferLength = width;

        // write in frame information for recording/playback
        axsunData->frameCount = gDaqCounter;
        axsunData->timeStamp = fileTimer.elapsed();;
        axsunData->milliseconds = 30;

        gDaqCounter++;

        yieldCurrentThread();

        return true;
    }
    return false;
}

bool DAQ::getData()
{
    uint32_t imaging, last_packet, last_frame, last_image, dropped_packets, frames_since_sync;  // for axGetStatus()
    request_prefs_t prefs{ };
    image_info_t info{ };
    static int32_t counter = 0;
    char message[512] = {};

    bool success{false};

    try {
        auto retval = axGetStatus(session, &imaging, &last_packet, &last_frame, &last_image, &dropped_packets, &frames_since_sync);
        if(retval != AxErr::NO_AxERROR) {
            throw retval;
        }

        if (imaging) {
            // get image info on most recent image enqueued in the buffer
            if (auto retval = axGetImageInfo(session, 0, &info); retval != AxErr::NO_AxERROR) {
                // print error details
                axGetErrorString(retval, message);
                LOG2(int(retval), message);
                success = true;
            }
            else {
                // if no errors, configure the request preferences and then request the image for display
                prefs.request_mode = AxRequestMode::RETRIEVE_TO_CALLER;
                prefs.which_window = 1;

                OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

                retval = axRequestImage(session, info.image_number, prefs, 0, axsunData->acqData, &info);

                // and print some statistics to the console (every 25th iteration)
                if (!(counter % 25)) {
                    //std::cout << "Image number: " << info.image_number << ", Width: " << info.width <<", Dropped Packets: " << dropped_packets;
                    LOG3(info.image_number, info.width, dropped_packets);
                    //if (info.force_trig) std::cout << " *** TRIGGER TOO SLOW, FORCE TRIGGERED ***";
                    //if (info.trig_too_fast) std::cout << " *** TRIGGER TOO FAST ***";
                    //std::cout << '\n';
                    LOG2(info.force_trig, info.trig_too_fast);
                }
            }
        }
        else {
            if (!(counter % 50)) {  // print every 50th iteration
                //std::cout << "Imaging is off. Turn on laser emission and set DAQ to Imaging On mode.\n";
                QString errorMessage("Imaging is off. Turn on laser emission and set DAQ to Imaging On mode.");
                LOG3(imaging, counter, errorMessage);
            }
        }

        // increment loop counter
        counter++;

    }  catch (const AxErr& e) {
        axGetErrorString(e, message);
        LOG1(message);
    }
    catch (...) {
        const QString errorMessage("***** UNKNOWN ERROR. Program terminating.");
        LOG1(errorMessage);
    }

    return success;
}
/*
 * startDaq
 *
 * Start the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::startDaq()
{
    AxErr success = AxErr::NO_AxERROR;
    char message[512];      // variable for getting string output from axGetMessage() and axGetErrorString()

    try {

        success = axStartSession(&session, 50);    // Start Axsun engine session
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
            throw success;
        } else {
            session0 = session;
        }
//        AxsunOCTCapture AOC(500);

        const int framesUntilForceTrig {35};
        /*
         * The number of frames for which the driver will wait for a Image_sync signal before timing out and entering Force Trigger mode.
         * Defaults to 24 frames at session creation.  Values outside the range of [2,100] will be automatically coerced into this range.
         * 35 * 256 = 8960 A lines
         */
//        msleep(100);
        success = axSetTrigTimeout(session, framesUntilForceTrig * 2);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }

//        msleep(100);
        success = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
//        msleep(100);
        success = axGetMessage(session, axMessage );
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        LOG1(axMessage)
    }
    catch (const AxErr& e) {
        axGetErrorString(e, message);
        LOG1(message);
    }
    catch(...){
        LOG0
    }
    const bool isSuccess(success == AxErr::NO_AxERROR);

    LOG1(isSuccess);

    return isSuccess;
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
    char message[512];      // variable for getting string output from axGetMessage() and axGetErrorString()

    try{
        success = axStopSession(session);    // Stop Axsun engine session
        if(success != AxErr::NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
            throw success;
        }
    }
    catch (const AxErr& e) {
        axGetErrorString(e, message);
        LOG1(message);
    }
    catch(...){
        LOG0
    }

    return success == AxErr::NO_AxERROR;
}

void DAQ::setLaserDivider()
{
//    if(m_numberOfConnectedDevices == 2)
    {
        const int subsamplingFactor = m_subsamplingFactor;
        if( subsamplingFactor > 0  && subsamplingFactor <= 4 )
        {
            LOG1(subsamplingFactor)
//            AxErr success = axSetSubsamplingFactor(subsamplingFactor,0);
//            if(success != AxErr::NO_AxERROR){
//                logAxErrorVerbose(__LINE__, success);
//            }
            auto success = axGetMessage( session, axMessage );
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
