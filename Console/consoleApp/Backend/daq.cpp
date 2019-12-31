///*
// * daq.cpp
// *
// * High-level interface to the AlazarTech DAQ.  This object creates
// * the thread for collecting and processing data from the DAQ and makes
// * the processed data available to the rest of the software.
// *
// * Author: Dennis W. Jackson
// *
// * Copyright (c) 2009-2018 Avinger, Inc.
// *
// */
//#include <QDebug>
//#include <QApplication>
//#include <QDebug>
//#include <QMutex>
//#include "daq.h"
//#include "defaults.h"
//#include "ipp.h"
//#include "profiler.h"
//#include "daqSettings.h"
//#include "deviceSettings.h"
//#include "logger.h"
//#include "buildflags.h"
//#include "util.h"
//#include "AlazarCmd.h"
//#include "AlazarApiFunction.h"
//#include "dsp.h"
//#include "daqSettings.h"
//#include "defaults.h"
//#include "../Utility/daqDataConsumer.h"
//#include "octFile.h"
//#include "playbackmanager.h"
//#include "theglobals.h"


//// Use a mutex to prevent multiple write-access the global data. This mutex
//// is locked for block writes to the data and is kept as long as the thread
//// uses the memory.

///////////////////////////////////////

///*
// * Constructor
// */
//DAQ::DAQ( void )
//{
//    isConfigured     = false;
//    isRunning        = false;

//    hBoard           = NULL;
//    recordToRead     = 1;

//    recordsPerBuffer = 0;
//    bytesPerBuffer   = 0;
//    bytesPerRecord   = 0;
//    channelCount     = 0;
//    frameCount       = 0;

//    triggerTimer.start(); // clocking the daq speed

//    /*
//     * Load the settings. This is a keyed file and will be verified by the init process,
//     * if enabled.
//     */
//    DaqSettings &settings = DaqSettings::Instance();

//    if( !settings.init() )
//    {
//        LOG( WARNING, "Unable to load DAQ Settings." );
//    }

//    dsp  = NULL;
//}

///*
// * Destructor
// */
//DAQ::~DAQ()
//{
//    shutdown();
//}

///*
// * init
// *
// * Initialize the hardware common to both types of DAQs
// */
//void DAQ::init( void )
//{
//    qDebug() << "DAQ::Init() start";

//    // make sure card is in a good state
//    hBoard = AlazarGetBoardBySystemID( 1, 1 );
//    AlazarAbortAsyncRead( hBoard );// abort any ongoing aync reads if the card was left in a weird state

//    LOG1(TheGlobals::instance()->getGDaqRawDataBuffersPosted())

//    emit updateCatheterView();

//    qDebug() << "DAQ::Init() end";
//}

///*
// * run
// *
// * Data-gathering and processing thread.  This is the main loop for the DAQ.
// */
//void DAQ::run( void )
//{
//    while( !isConfigured ) // XXX
//    {
//        qDebug() << "DAQ::run -- waiting for isConfigured";
//        yieldCurrentThread();
//    }

//    qDebug() << "DAQ::run() start";

//    // prevent multiple, simultaneous starts
//    if( !isRunning )
//    {
//        deviceSettings &devSettings = deviceSettings::Instance();
//        bool isHighSpeedDevice = devSettings.current()->isHighSpeed();

//        qDebug() << "Thread: DAQ::run start";
//        isRunning = true;

//        // get control of the global data
//        gFrameWriteMutex.lock();

//        RETURN_CODE status = ApiSuccess;
//        if( isHighSpeedDevice )
//        {
//            // Start the DSP thread
//            dsp->start();

//            // start the DMA transfers
//            status = AlazarStartCapture( hBoard );
//            qDebug() << "AlazarStartCapture() status = " << AlazarErrorToText( status );
//        }

//        // set the brightness and contrast when the daq starts up
//        if( isHighSpeedDevice )
//        {
//            setBlackLevel( BrightnessLevels_HighSpeed.defaultValue );
//            setWhiteLevel( ContrastLevels_HighSpeed.defaultValue );
//        }
//        else
//        {
//            setBlackLevel( BrightnessLevels_LowSpeed.defaultValue );
//            setWhiteLevel( ContrastLevels_LowSpeed.defaultValue );
//        }

//        while( isRunning )
//        {
//            TIME_THIS_SCOPE( daq_run );

//            getData(); //lcv

//            // Low speed devices process the data immediately
//            if( !isHighSpeedDevice )
//            {
//                dsp->processData();
//            }

//            // Rough frames/second counter
//            frameCount++;
//            if( triggerTimer.elapsed() > 1000 )
//            {
//#if ENABLE_FRAME_COUNTERS_TO_DEBUG
//                qDebug() << "DAQ frameCount/s:" << frameCount;
//#endif
//                emit frameRate( frameCount );

//                // reset for the next measurement
//                frameCount = 0;
//                triggerTimer.restart();
//            }

//            // threads do not handle events by default (timer expiration). Do so explicitly.
//            QApplication::processEvents();
//            yieldCurrentThread();
//        }

//        if( isHighSpeedDevice )
//        {
//            // abort any ongoing async reads
//            status = AlazarAbortAsyncRead( hBoard );
//            // stop the DSP thread
//            dsp->stop();
//            dsp->wait();
//        }

//        // release control of the global data
//        gFrameWriteMutex.unlock();

//        qDebug() << "Thread: DAQ::run stop";
//    }
//}


///*
// * checkSDKVersion
// *
// * Used to verify the installed SDK on the PC matches
// * what it was developed for.
// */
//bool DAQ::checkSDKVersion( void )
//{
//    U8 verMajor( 0 );
//    U8 verMinor( 0 );
//    U8 verRevision( 0 );

//    AlazarGetSDKVersion( &verMajor, &verMinor, &verRevision );

//    LOG( INFO, QString( "DAQ SDK Version: %1.%2.%3" ).arg( verMajor ).arg( verMinor ).arg( verRevision ) );

//    bool isOK = ( ( verMajor == SDK_Major ) &&
//                  ( verMinor == SDK_Minor ) );

//    if ( !isOK )
//    {
//        LOG( WARNING, QString( "Expected SDK Version: %1.%2" ).arg( SDK_Major ).arg( SDK_Minor ) );
//    }

//    return( isOK );
//}

///*
// * checkDriverVersion
// *
// * Used to verify the installed driver on the PC matches
// * what it was developed for.
// */
//bool DAQ::checkDriverVersion( void )
//{
//    U8 verMajor( 0 );
//    U8 verMinor( 0 );
//    U8 verRevision( 0 );

//    AlazarGetDriverVersion( &verMajor, &verMinor, &verRevision );

//    LOG( INFO, QString( "DAQ Driver Version: %1.%2.%3" ).arg( verMajor ).arg( verMinor ).arg( verRevision ) );

//    bool isOK = ( ( verMajor == Driver_Major ) &&
//                  ( verMinor == Driver_Minor ) );

//    if( !isOK )
//    {
//        LOG( WARNING, QString( "Expected Driver Version: %1.%2" ).arg( Driver_Major ).arg( Driver_Minor ) );
//    }

//    return( isOK );
//}

///*
// * queryBoards
// *
// * Verify and log some low-level info about the Alazar boards
// */
//int DAQ::queryBoards( int numOfBoards )
//{
//    U8 mj1, mj2, mn1, mn2, r1, r2;
//    int retVal = 0;
//    U32 serialNum;
//    U32 boardType;
//    U32 aspocType;
//    U32 memSize;
//    U32 sampleSize;
//    int fpgaMajorVer;
//    int fpgaMinorVer;

//    if ( numOfBoards == 0 )
//    {
//        qDebug( "No AlazarTech devices were detected.\n" );
//        retVal = 0;
//    }
//    else
//    {
//        AlazarGetSDKVersion( &mj1, &mn1, &r1 );
//        qDebug( "Alazar DAQ SDK Version: %i.%i.%i", mj1, mn1, r1 );

//        AlazarGetDriverVersion( &mj2, &mn2, &r2 );
//        qDebug( "Alazar DAQ Driver Version: %i.%i.%i", mj2, mn2, r2 );

//        for ( int i = 0; i < numOfBoards; i++ )
//        {
//            qDebug( "Alazar DAQ Board No. %i", i + 1 );
//            LOG( INFO, QString( "Alazar DAQ Board No. %1" ).arg( i + 1 ) );

//            AlazarQueryCapability( AlazarGetBoardBySystemID( 1, i + 1 ), GET_SERIAL_NUMBER, 0, &serialNum );
//            qDebug(" - Serial No: %i", serialNum );
//            LOG( INFO, QString( " - Serial No: %1" ).arg( serialNum ) );

//            AlazarQueryCapability( AlazarGetBoardBySystemID( 1, i + 1 ), BOARD_TYPE, 0, &boardType );
//            qDebug(" - Board Type: %i", boardType );
//            LOG( INFO, QString( " - Board Type: %1" ).arg( boardType ) );

//            AlazarQueryCapability( AlazarGetBoardBySystemID( 1, i + 1 ), ASOPC_TYPE, 0, &aspocType );
//            fpgaMajorVer = (aspocType >> 16) & 0xff;
//            fpgaMinorVer = (aspocType >> 24) & 0xf;
//            qDebug( " - ASOPC: %08XL", aspocType );
//            LOG( INFO, QString( " - ASOPC: %1" ).arg( aspocType ) );

//            QString fwVersion = QString( "   - FPGA version %1.%2" ).arg( fpgaMajorVer ).arg( fpgaMinorVer );
//            qDebug() << fwVersion;
//            LOG( INFO, fwVersion );

//            // verify the FPGA is compatible.  Do not continue on if it is not.
//            if( ( ( fpgaMajorVer * 100 ) + fpgaMinorVer ) < ( ( FPGA_Major * 100 ) + FPGA_Minor ) )
//            {
//                // call error handler directly since this is called from the system Init
//                displayFailureMessage( tr( "DAQ FPGA version is incompatible with this release." ), true );
//            }

//            AlazarQueryCapability( AlazarGetBoardBySystemID( 1, i + 1 ), MEMORY_SIZE, 0, &memSize );
//            qDebug(" - Memory Size: %i", memSize );
//            LOG( INFO, QString( " - Memory Size: %1" ).arg( memSize ) );

//            AlazarQueryCapability( AlazarGetBoardBySystemID( 1, i + 1 ), SAMPLE_SIZE, 0, &sampleSize );
//            qDebug(" - Sample Size: %i", sampleSize );
//            LOG( INFO, QString( " - Sample Size: %1" ).arg( sampleSize ) );

//            /*
//             * Check for FIFO-only capabilities.  DAQ lost it's mind and doesn't know
//             * it has memory on-board.
//             */
//            if( memSize == 0 )
//            {
//                // fatal error
//                displayFailureMessage( tr( "\nPlease restart a case or restart Lightbox.\nDAQ error (FIFO only)." ), true );
//            }

//            // Confirm the hardware
//            if ( boardType == ATS9462 )
//            {
//                retVal++;
//            }
//        }
//    }
//    return retVal;
//}

///*
// * isDaqPresent
// *
// * Verify the hardware is installed in the machine
// */
//bool DAQ::isDaqPresent( void )
//{
//    LOG( INFO, QString( "DAQ Alazar boards found: %1" ).arg( AlazarBoardsFound() ) );
//    return( queryBoards( AlazarBoardsFound() ) >= 1 );
//}

///*
// * shutdown
// *
// * Free resources and shut down the hardware
// */
//void DAQ::shutdown( void )
//{
//    LOG( INFO, "DAQ shutdown" );

//    // make sure card is in a good state
//    AlazarAbortAsyncRead( hBoard );// abort any ongoing aync reads if the card was left in a weird state

//    // release the DSP
//    if( dsp != NULL )
//    {
//        delete dsp;
//        dsp = NULL;
//    }

//    // release raw data storage space
//    TheGlobals::instance()->freeDaqRawDataBuffer();

//    // Free the global memory
//    TheGlobals::instance()->freeFrameData();

//    /*
//     * No need to call AlazarClose. The API does it automatically
//     * when the program shuts down and calling it for a running
//     * program causes the board to not return a handle.  Per email
//     * with Nigel at AlazarTech, 1 Mar 2012.
//     */
//}

///*
// * getDaqLevel
// *
// * Convert the DAQ level constants to strings for display. These values are
// * hard-coded in the AlazarTech API.
// */
//QString DAQ::getDaqLevel( void )
//{
//    DaqSettings &settings = DaqSettings::Instance();

//    QString daqLevel;

//    switch( settings.getChAInputRange() )
//    {
//    case INPUT_RANGE_PM_20_MV:  daqLevel = "20 mV";   break;
//    case INPUT_RANGE_PM_40_MV:  daqLevel = "40 mV";   break;
//    case INPUT_RANGE_PM_50_MV:  daqLevel = "50 mV";   break;
//    case INPUT_RANGE_PM_80_MV:  daqLevel = "80 mV";   break;
//    case INPUT_RANGE_PM_100_MV: daqLevel = "100 mV";  break;
//    case INPUT_RANGE_PM_200_MV: daqLevel = "200 mV";  break;
//    case INPUT_RANGE_PM_400_MV: daqLevel = "400 mV";  break;
//    case INPUT_RANGE_PM_500_MV: daqLevel = "500 mV";  break;
//    case INPUT_RANGE_PM_800_MV: daqLevel = "800 mV";  break;
//    case INPUT_RANGE_PM_1_V:    daqLevel = "1 V";     break;
//    case INPUT_RANGE_PM_2_V:    daqLevel = "2 V";     break;
//    case INPUT_RANGE_PM_4_V:    daqLevel = "4 V";     break;
//    case INPUT_RANGE_PM_5_V:    daqLevel = "5 V";     break;
//    case INPUT_RANGE_PM_8_V:    daqLevel = "8 V";     break;
//    default:                    daqLevel = "Unknown"; break;
//    }
//    return daqLevel;
//}

//void DAQ::stop()
//{
//    isRunning = false;
//}


///*
// * enableAuxTriggerAsTriggerEnable
// * R&D only
// */
//void DAQ::enableAuxTriggerAsTriggerEnable( bool state )
//{
//    RETURN_CODE status;

//    qDebug() << "Use AUX Trigger Enable" << state;

//    if( state )
//    {
//        status = AlazarConfigureAuxIO( hBoard, AUX_IN_TRIGGER_ENABLE, TRIGGER_SLOPE_POSITIVE );
//    }
//    else
//    {
//        status = AlazarConfigureAuxIO( hBoard, AUX_INPUT_AUXILIARY, TRIGGER_SLOPE_POSITIVE );
//    }

//    if( status != ApiSuccess )
//    {
//        sendWarning( tr( "Could not toggle AUX hardware" ) );
//    }
//}

//long DAQ::getRecordLength() const
//{
//    DaqSettings &settings = DaqSettings::Instance();

//    return long(settings.getRecordLength());
//}

///*
// * pause
// *
// * Keeps the DAQ running but turns off data processing.  To the frontend, this
// * looks like the DAQ is stopped.  This is a lightweight method of stopping
// * acquistion without tearing down the entire DAQ and DSP.  It also keeps
// * the Alazar API from going off into the weeds with DMA transfers.
// */
//void DAQ::pause( void )
//{
//    // DSP runs in a thread only when the High Speed device is selected
//    deviceSettings &devSettings = deviceSettings::Instance();
//    if( devSettings.current()->isHighSpeed() )             // XXX: can we move the low speed to a thread for consistency?
//    {
//        if( dsp != NULL )
//        {
//            dsp->stop();
//        }
//    }
//}

///*
// * resume
// *
// * Turn the DSP back on to restart data processing.
// */
//void DAQ::resume( void )
//{
//    // DSP runs in a thread only when the High Speed device is selected
//    deviceSettings &devSettings = deviceSettings::Instance();
//    if( devSettings.current()->isHighSpeed() )             // XXX: can we move the low speed to a thread for consistency?
//    {
//        if( dsp != NULL )
//        {
//            dsp->start();
//        }
//    }
//}

