#include "daq.h"
#include <QApplication>
#include <QDebug>
//#include "controller.h"
#include "logger.h"
#include <algorithm>

#ifdef WIN32
//#include "stdafx.h"     // Axsun includes
extern "C" {
#include "AxsunOCTCapture.h"
}
#endif

#define PCIE_MODE 0         // PCIE or Ethernet mode for non-synthetic
#define USE_LVDS_TRIGGER 1  // Use LVDS trigger (or LVCMOS)

#define UINT16_MAX_VAL 65535
#define NUM_OF_FRAME_BUFFERS 4
#define ALLOCATED_OVERRUN_BUFFER_SIZE ( 256 * FFT_DATA_SIZE )  // Overrun buffer for Ocelot Mode

namespace{
int gFrameNumber = 0;
int gDaqCounter = 0;
size_t gBufferLength;
OCTFile::OctData_t gFrameData[ NUM_OF_FRAME_BUFFERS ];
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

    // allocate frame memory without overrun buffer
    for( int i = 0; i < NUM_OF_FRAME_BUFFERS; i++ )
    {
        gFrameData[ i ].acqData = (uint8_t *)malloc( MAX_ACQ_IMAGE_SIZE );
        gFrameData[ i ].dispData = (unsigned char *)malloc( SECTOR_SIZE_B );
    }

    if( !startDaq() )
    {
        qDebug() << "DAQ: Failed to start DAQ";
    }
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

        int frameCount = NUM_OF_FRAME_BUFFERS - 1;
        int loopCount = NUM_OF_FRAME_BUFFERS - 1;
        LOG2(frameCount,loopCount)
        qDebug() << "***** Thread: DAQ::run()";
        while( isRunning )
        {
            // Rough lines/second counter  XXX
            frameCount++;
            loopCount++;
            if( frameTimer.elapsed() > 1000 )
            {
//                qDebug() << "                       DAQ frameCount/s:" << frameCount << " width:" << gBufferLength << " frame:" << gDaqCounter;
                emit fpsCount( frameCount );
                emit linesPerFrameCount( (int)gBufferLength );
                emit missedImagesCount( missedImgs );
                frameCount = 0;
                frameTimer.restart();
            }

            // get data and only procede if the image is new.
            if( getData() )
            {
                gFrameNumber = loopCount % NUM_OF_FRAME_BUFFERS;
                if( scanWorker->isReady )
                {
                    static int count{0};
                    OCTFile::OctData_t& axsunData = gFrameData[ gFrameNumber ];
//                    scanWorker->warpData( &gFrameData[ gFrameNumber ], gBufferLength );
//                    emit updateSector(&gFrameData[ gFrameNumber ]);
                    scanWorker->warpData( &axsunData, gBufferLength );
                    emit updateSector(&axsunData);
                }
            }
            else
            {
                // since it was incremented above, decrement upon a failed acquisition.
                frameCount--;
                loopCount--;
            }
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
    bool retVal = false;

    uint32_t imaging, last_packet_in,last_frame_in, last_image_in, dropped_packets, frames_since_sync;
    dropped_packets = 0;
    uint32_t required_buffer_size = 0;
    uint32_t returned_image_number = 0;
    int32_t width = 0;
    int32_t height = 0;
    AxDataType data_type = U8;
    uint32_t returned_image;
    uint8_t force_trig;
    uint8_t trig_too_fast;

    axRetVal = axGetStatus(session, &imaging, &last_packet_in, &last_frame_in, &last_image_in, &dropped_packets, &frames_since_sync );
//    qDebug() << "***** axGetStatus: " << axRetVal << "last_packet_in: " << last_packet_in;

    axRetVal = axGetImageInfoAdv(session, -1, &returned_image_number, &height, &width, &data_type, &required_buffer_size, &force_trig, &trig_too_fast );
//    qDebug() << "***** axGetImageInfoAdv: " << axRetVal << "Image number: " << returned_image_number;

    if( returned_image_number > (lastImageIdx + 1) )
    {
        qDebug() << "Missed images: " << ( returned_image_number - lastImageIdx - 1 );
        missedImgs = (returned_image_number - lastImageIdx - 1);
        LOG1(missedImgs)
    }
    else
    {
        missedImgs = 0;
    }

    if( missedImgs > 0 )
    {
        emit missedImagesCount( missedImgs );
    }


    if( returned_image_number <= lastImageIdx )
    {
        return false;
    }
    lastImageIdx = returned_image_number;

    if( ( axRetVal != -9999 ) && ( axRetVal != -9994 ) && ( force_trig != 1 ) )
    {
        axRetVal = axRequestImage( session,
                                   returned_image_number,
                                   &returned_image,
                                   &height,
                                   &width,
                                   &data_type,
                                   gFrameData[ gFrameNumber ].acqData,
                                   MAX_ACQ_IMAGE_SIZE );
        gBufferLength = width;

        // write in frame information for recording/playback
        gFrameData[ gFrameNumber ].frameCount = gDaqCounter;
        gFrameData[ gFrameNumber ].timeStamp = fileTimer.elapsed();
        gFrameData[ gFrameNumber ].milliseconds = 30;
        gDaqCounter++;

        if( axRetVal == 0 )
        {
            return true;
        }
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

        axRetVal = axStartSession(&session, 200);    // Start Axsun engine session
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
        axRetVal = axWriteFPGAreg( session, 2, 0x0604 ); // Write FPGA register 2 to 0x0604.  Use LVCMOS trigger input
        axGetMessage( session, axMessage );
        qDebug() << "axWriteFPGAreg: " << retVal << " message:" << axMessage;
#endif
//    setLaserDivider(LASER_SCAN_DIVIDER);
    } catch (...) {
        qDebug() << "Axsun Error" ;
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
    int temp = divider + 1;
    if( temp > 0  && temp <= 4 )
    {
#if PCIE_MODE
        axRetVal = axWriteFPGAreg( session, 60, divider ); // Write FPGA register 6 ( Aline rate 100kHz / (parm +1) )
#else
        axRetVal = axWriteFPGAreg( session, 60, divider ); // Write FPGA register 6 ( Aline rate 100kHz / (parm +1) )
#endif
        axGetMessage( session, axMessage );
        qDebug() << "***** axSetFPGARegister: " << temp << " message:" << axMessage;
        qDebug() << "Setting laser divider to:" << divider + 1;
    }
}

void DAQ::setDisplay(float angle, int direction)
{
    qDebug() << "got to setDisplay" << angle << direction;
    emit setDisplayAngle( angle, direction );
}
