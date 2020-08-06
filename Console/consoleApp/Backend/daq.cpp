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

        int loopCount = NUM_OF_FRAME_BUFFERS - 1;
        LOG2(loopCount, m_decimation)
        qDebug() << "***** Thread: DAQ::run()";
        while( isRunning )
        {

            // get data and only procede if the image is new.
            if( getData() )
            {
                gFrameNumber = ++loopCount % NUM_OF_FRAME_BUFFERS;
                auto* sm =  SignalModel::instance();
                OCTFile::OctData_t* axsunData = sm->getOctData(gFrameNumber);
                sm->setBufferLength(gBufferLength);
//                if( scanWorker->isReady )
                {
//                    scanWorker->warpData( axsunData, gBufferLength );

                    emit updateSector(axsunData);
                }
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

bool DAQ::getData2( )
{
    int64_t requested_image_number = -1;
    static int32_t lostImageCount = 0;
    static uint32_t imageCount = 0;
    static uint32_t sreturned_image_number = -1;
    float lostImagesInPercent = 0.0f;
    uint32_t returned_image_number = 0;
    int32_t width = 0;
    int32_t height = 0;
    AxDataType data_type = U8;

    OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);
    AxErr axSuccess = axRequestImage( session,
                               requested_image_number,
                               &returned_image_number,
                               &height,
                               &width,
                               &data_type,
                               axsunData->acqData,
                               MAX_ACQ_IMAGE_SIZE );

    if(axSuccess != NO_AxERROR || width < 6000){
        return false;
    }
    if(sreturned_image_number == -1){
        sreturned_image_number = returned_image_number;
        lastImageIdx = returned_image_number - 1;
        LOG4(m_count, returned_image_number, lostImageCount, lostImagesInPercent)
//        LOG4(errorcount,required_buffer_size,height, width)
    }

    if(axRetVal == NO_AxERROR && returned_image_number != sreturned_image_number){
        sreturned_image_number = returned_image_number;
        ++m_count;
        ++imageCount;
        if(m_decimation && (m_count % m_decimation == 0)){
            lostImagesInPercent =  100.0f * lostImageCount / imageCount;
            LOG4(m_count, returned_image_number, lostImageCount, lostImagesInPercent)
//            LOG4(errorcount,required_buffer_size,height, width)
        }
        if( returned_image_number > (lastImageIdx + 1) ){
           lostImageCount += returned_image_number - lastImageIdx - 1;
        }
        lastImageIdx = returned_image_number;
     }

    gBufferLength = width;

    // write in frame information for recording/playback
    axsunData->frameCount = gDaqCounter;
    axsunData->timeStamp = fileTimer.elapsed();;
    axsunData->milliseconds = 30;

    gDaqCounter++;

//    yieldCurrentThread();

    return true;
}

/*
 * getData
 */
bool DAQ::getData( )
{
    bool retVal = false;

    uint32_t required_buffer_size = 0;
    uint32_t returned_image_number = 0;
    static int32_t sreturned_image_number = -1;
    static int32_t lostImageCount = 0;
    static uint32_t imageCount = 0;
    static uint32_t errorcount = 0;
    float lostImagesInPercent = 0.0f;
    int32_t width = 0;
    int32_t height = 0;
    AxDataType data_type = U8;
    uint32_t returned_image = 0;
    uint8_t force_trig = 0;
    uint8_t trig_too_fast = 0;
    static std::map<AxErr,int> errorTable;
    static int force_trigCount = 0;
    static int trig_too_fastCount = 0;

//    axRetVal = axGetStatus(session, &imaging, &last_packet_in, &last_frame_in, &last_image_in, &dropped_packets, &frames_since_sync );
////    qDebug() << "***** axGetStatus: " << axRetVal << "last_packet_in: " << last_packet_in;

    int64_t requestedImageNumber = -1;

//    msleep(1);
    axRetVal = axGetImageInfoAdv(session, requestedImageNumber, &returned_image_number, &height, &width, &data_type, &required_buffer_size, &force_trig, &trig_too_fast );
//    qDebug() << "***** axGetImageInfoAdv: " << axRetVal << "Image number: " << returned_image_number;

    bool isReturn = false;

    if(axRetVal != NO_AxERROR){
        AxErr errorNum = axRetVal;
//        if(auto it = errorTable.find(errorNum) != errorTable.end()){
//            errorTable[errorNum]++;
//        } else {
//            errorTable[errorNum] = 1;
//        }
        ++errorcount;
        isReturn = true;
    }

    if( force_trig == 1){
        force_trigCount++;
        isReturn = true;
    }

    if( trig_too_fast == 1){
        trig_too_fastCount++;
        isReturn = true;
    }

    if(isReturn){
        return false;
    }


    if(sreturned_image_number == -1){
        sreturned_image_number = returned_image_number;
        lastImageIdx = returned_image_number - 1;
        force_trigCount = 0;
        LOG4(m_count, returned_image_number, lostImageCount, lostImagesInPercent)
        LOG4(errorcount,required_buffer_size,height, width)
//        LOG3(errorTable.size(), force_trigCount, trig_too_fastCount)
        LOG2(force_trigCount, trig_too_fastCount)
    }

    if(axRetVal == NO_AxERROR && returned_image_number != sreturned_image_number){
        sreturned_image_number = returned_image_number;
        ++m_count;
        ++imageCount;
        if(m_decimation && (m_count % m_decimation == 0)){
            lostImagesInPercent =  100.0f * lostImageCount / imageCount;
            LOG4(m_count, returned_image_number, lostImageCount, lostImagesInPercent)
            LOG4(errorcount,required_buffer_size,height, width)
            LOG2(force_trigCount, trig_too_fastCount)
//            LOG3(errorTable.size(), force_trigCount, trig_too_fastCount)
//            if(errorTable.size() >= 1){
//                auto it = errorTable.begin();
//                for(int i = 0; i < int(errorTable.size()); ++i ){
//                    AxErr error = it->first;
//                    int errorCode = int(error);
//                    auto errorCount = it->second;
//                    char errorMsg[512];
//                    axGetErrorString(error, errorMsg);
//                    LOG3(errorCode, errorCount, errorMsg)
//                    ++it;
//                }
//             }
        }
        if( returned_image_number > (lastImageIdx + 1) ){
           lostImageCount += returned_image_number - lastImageIdx - 1;
        }
     }

    if( returned_image_number <= lastImageIdx )
    {
        return false;
    }

    lastImageIdx = returned_image_number;

//    return true;

    OCTFile::OctData_t* axsunData = SignalModel::instance()->getOctData(gFrameNumber);

//    if( ( axRetVal != DATA_NOT_FOUND_IN_BUFFER ) && ( axRetVal != DATA_ALLOCATION_TOO_SMALL ) && ( force_trig != 1 ) ) - meaning of original
//    if( ( axRetVal != -9999 ) && ( axRetVal != -9994 ) && ( force_trig != 1 ) ) // original

    if((axRetVal == NO_AxERROR) && (force_trig != 1)) //try 1
    {
        axRetVal = axRequestImage( session,
                                   returned_image_number,
                                   &returned_image,
                                   &height,
                                   &width,
                                   &data_type,
                                   axsunData->acqData,
                                   MAX_ACQ_IMAGE_SIZE );
        gBufferLength = width;

        // write in frame information for recording/playback
        axsunData->frameCount = gDaqCounter;
        axsunData->timeStamp = fileTimer.elapsed();;
        axsunData->milliseconds = 30;

        gDaqCounter++;

        if( axRetVal == 0 )
        {
            return true;
        }
        yieldCurrentThread();
    }
    else
    {
        qDebug() << "Data Not Ready - force_trig:" << force_trig;
    }

    return retVal;
}

/*
 * startDaq
 *
 * Start the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::startDaq()
{
    qDebug() << "***** DAQ::startDaq()";
    axRetVal = NO_AxERROR;

    try {

        axRetVal = axStartSession(&session, 50);    // Start Axsun engine session
        if(axRetVal != NO_AxERROR){
            char message_out[512];
            axGetErrorString(axRetVal, message_out);
            LOG1(message_out)
        }
        axRetVal = axSetTrigTimeout(session, 100);
        if(axRetVal != NO_AxERROR){
            char message_out[512];
            axGetErrorString(axRetVal, message_out);
            LOG1(message_out)
        }

#if PCIE_MODE
        axRetVal = axSelectInterface(session, AxInterface::PCI_EXPRESS);
        axRetVal = axImagingCntrlPCIe(session, -1);
        axRetVal = axPipelineMode(session, EIGHT_BIT);
#else
        axRetVal = axSelectInterface(session, AxInterface::GIGABIT_ETHERNET);
#endif

#if USE_LVDS_TRIGGER
        axGetMessage(session, axMessage );
        qDebug() << "message:" << axMessage;
#else
//        axRetVal = axWriteFPGAreg( session, 2, 0x0604 ); // Write FPGA register 2 to 0x0604.  Use LVCMOS trigger input
        axGetMessage( session, axMessage );
        qDebug() << "axWriteFPGAreg: " << retVal << " message:" << axMessage;
#endif
        const int laserDevider{1};
        LOG1(laserDevider)
        setLaserDivider(laserDevider);
    } catch (...) {
        qDebug() << "Axsun Error" ;
        LOG1("Axsun Error")
    }

    return axRetVal == NO_AxERROR;
}

/*
 * stopDaq
 *
 * Stop the Axsun DAQ based on PCIe or Ethernet mode.
 */
bool DAQ::shutdownDaq()
{
    qDebug() << "***** DAQ::shutdownDaq()";
    axRetVal = NO_AxERROR;
    axRetVal = axStopSession(session);    // Stop Axsun engine session
    return axRetVal == NO_AxERROR;
}

void DAQ::setLaserDivider( int divider)
{
    const int subsamplingFactor = divider + 1;
    if( subsamplingFactor > 0  && subsamplingFactor <= 4 )
    {
#if PCIE_MODE
//        axRetVal = axWriteFPGAreg( session, 60, divider ); // Write FPGA register 6 ( Aline rate 100kHz / (parm +1) )
#else
//        axRetVal = axWriteFPGAreg( session, 60, divider ); // Write FPGA register 6 ( Aline rate 100kHz / (parm +1) )
#endif
        LOG2(subsamplingFactor, divider)
//        axSetSubsamplingFactor(subsamplingFactor,0);
        axGetMessage( session, axMessage );
        qDebug() << "***** axSetFPGARegister: " << subsamplingFactor << " message:" << axMessage;
        qDebug() << "Setting laser divider to:" << divider + 1;
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
