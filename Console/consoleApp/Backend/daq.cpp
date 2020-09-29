#include "daq.h"
#include <QApplication>
#include <QDebug>
//#include "controller.h"
#include "logger.h"
#include <algorithm>
#include "signalmodel.h"
#include "Utility/userSettings.h"

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

//static uint8_t gDaqBuffer[ 256 * FFT_DATA_SIZE ];

/*
 * Constructor
 */
DAQ::DAQ()
{
    isRunning    = false;
    scanWorker   = new ScanConversion();
    lastImageIdx = 0;
    missedImgs   = 0;
    lapCounter   = 0;   // Ocelot lap counter
    gFrameNumber = NUM_OF_FRAME_BUFFERS - 1;

    connect(this, SIGNAL(setDisplayAngle(float, int)), scanWorker, SLOT(handleDisplayAngle(float, int)) );

    if( !startDaq() )
    {
        qDebug() << "DAQ: Failed to start DAQ";
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
    LOG3(line, axErrorCode, errorVerbose)
}

DAQ::~DAQ()
{
}

void DAQ::init()
{

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
        qDebug() << "***** Thread: DAQ::run()";
        LOG1(axCountConnectedDevices())
        retval = axNetworkInterfaceOpen(1);
        if(retval != NO_AxERROR){
            char errorMsg[512];
            axGetErrorString(retval, errorMsg);
            LOG2(retval, errorMsg)
        }
        retval = axUSBInterfaceOpen(1);
        if(retval != NO_AxERROR){
            char errorMsg[512];
            axGetErrorString(retval, errorMsg);
            LOG2(retval, errorMsg)
        }
        while( isRunning )
        {

            // get data and only procede if the image is new.
            if( getData() )
            {
                gFrameNumber = ++loopCount % NUM_OF_FRAME_BUFFERS;
                auto* sm =  SignalModel::instance();
                OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
                sm->setBufferLength(gBufferLength);

                emit updateSector(axsunData);
            }
            yieldCurrentThread();
        }
    }
    if(shutdownDaq()){
        qDebug() << "Thread: DAQ::run stop";
    } else {
        qDebug() << "Thread: DAQ::run failed to shut down";
    }
}

/*
 * getData
 */
bool DAQ::getData( )
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
    AxDataType data_type = U8;
    uint32_t returned_image = 0;
    uint8_t force_trig = 0;
    uint8_t trig_too_fast = 0;
    static std::map<AxErr,int> errorTable;
    static int64_t force_trigCount = 0;
    static int64_t trig_too_fastCount = 0;
    AxErr success{NO_AxERROR};
    static AxErr sRetVal{NO_AxERROR};
    bool isReturnedImageNumberChanged{false};

    int64_t requestedImageNumber = -1;

    success = axGetImageInfoAdv(session, requestedImageNumber, &returned_image_number, &height, &width, &data_type, &required_buffer_size, &force_trig, &trig_too_fast );

    isReturnedImageNumberChanged = (success == NO_AxERROR) && (returned_image_number != sprevReturnedImageNumber);
    sprevReturnedImageNumber = returned_image_number;

    bool isReturn = false;

    if(success != sRetVal){
        sRetVal = success;
        if(success != NO_AxERROR){
            if(auto it = errorTable.find(success) != errorTable.end()){
                errorTable[success]++;
            } else {
                errorTable[success] = 1;
            }
            ++axErrorCount;
            isReturn = true;
        }
    }

    if(success != NO_AxERROR){
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
            LOG2(force_trigCount, trig_too_fastCount)
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
            LOG1(axCountConnectedDevices() )
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

        OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

        success = axRequestImage( session,
                                   returned_image_number,
                                   &returned_image,
                                   &height,
                                   &width,
                                   &data_type,
                                   axsunData->acqData,
                                   MAX_ACQ_IMAGE_SIZE );
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
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

/*
 * startDaq
 *
 * Start the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::startDaq()
{
    AxErr success = NO_AxERROR;

    try {

        success = axStartSession(&session, 50);    // Start Axsun engine session
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        const int framesUntilForceTrig {35};
        /*
         * The number of frames for which the driver will wait for a Image_sync signal before timing out and entering Force Trigger mode.
         * Defaults to 24 frames at session creation.  Values outside the range of [2,100] will be automatically coerced into this range.
         * 35 * 256 = 8960 A lines
         */
        msleep(100);
        success = axSetTrigTimeout(session, framesUntilForceTrig * 2);
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }

        msleep(100);
        success = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        msleep(100);
        success = axGetMessage(session, axMessage );
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        LOG1(axMessage)

        const int laserDivider{1};
        msleep(100);
        LOG1(laserDivider)
        setLaserDivider(laserDivider);
    } catch (...) {
        LOG1("Axsun init Error")
    }

    return success == NO_AxERROR;
}

/*
 * stopDaq
 *
 * Stop the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::shutdownDaq()
{
    qDebug() << "***** DAQ::shutdownDaq()";
    AxErr success = NO_AxERROR;

    success = axStopSession(session);    // Stop Axsun engine session
    if(success != NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
    }
    return success == NO_AxERROR;
}

void DAQ::setLaserDivider( int divider)
{

    AxErr success = axOpenAxsunOCTControl(true);
    if(success != NO_AxERROR){
        logAxErrorVerbose(__LINE__, success);
    }
    msleep(100);
    LOG1(axCountConnectedDevices())

    msleep(100);
    const int subsamplingFactor = divider + 1;
    if( subsamplingFactor > 0  && subsamplingFactor <= 4 )
    {
#if PCIE_MODE
        success = axWriteFPGAreg( session, 60, divider ); // Write FPGA register 6 ( Aline rate 100kHz / (parm +1) )
#endif
        LOG2(subsamplingFactor, divider)
        success = axSetSubsamplingFactor(subsamplingFactor,0);
//        success =  axSetFPGARegister(60,1,0);
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        success = axGetMessage( session, axMessage );
        if(success != NO_AxERROR){
            logAxErrorVerbose(__LINE__, success);
        }
        LOG1(axMessage);
    }
}

void DAQ::setDisplay(float angle, int direction)
{
    qDebug() << "got to setDisplay" << angle << direction;
    emit setDisplayAngle( angle, direction );
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
