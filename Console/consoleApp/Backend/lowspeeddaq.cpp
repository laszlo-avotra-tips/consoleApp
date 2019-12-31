///*
// * lowspeeddaq.cpp
// *
// * Implements the DAQ interface for low speed devices. Low speed devices
// * acquire and process single A-lines at a time and provide the A-lines
// * fully baked to the frontend.
// *
// * Author: Dennis W. Jackson
// *
// * Copyright (c) 2012-2018 Avinger, Inc.
// *
// */
//#include "lowspeeddaq.h"
//#include "deviceSettings.h"
//#include "logger.h"
//#include "buildflags.h"
//#include "profiler.h"
//#include "softwaredsp.h"
//#include "AlazarCmd.h"
//#include "AlazarApiFunction.h"
//#include "daqSettings.h"
//#include "theglobals.h"


//// Raw data from the DAQ hardware. Each element points to the start of Channel data

///*
// * constructor
// *
// * Default
// */
//LowSpeedDAQ::LowSpeedDAQ()
//{
//}


///*
// * destructor
// *
// * Default
// */
//LowSpeedDAQ::~LowSpeedDAQ()
//{
//}

///*
// * init
// *
// * Initialize the components of the DAQ necessary for low speed devices.
// * The Sofware DSP is created and initialized here.
// */
//void LowSpeedDAQ::init( void )
//{
//    qDebug() << "LowSpeedDAQ::init start";

//    // Call the common DAQ initialization
//    DAQ::init();

//    // Create the DSP
//    dsp = new SoftwareDSP();

//    if( dsp == NULL )
//    {
//        // fatal error
//        emit sendError( tr( "DSP thread could not be created." ) );
//    }

//    // Pass messages to/from the DSP
//    connect( this, SIGNAL(setBlackLevel(int)),    dsp, SLOT(setBlackLevel(int)) );
//    connect( this, SIGNAL(setWhiteLevel(int)),    dsp, SLOT(setWhiteLevel(int)) );
//    connect( this, SIGNAL(setInvertColors(bool)), dsp, SLOT(setInvertColors(bool)) );
//    connect( this, SIGNAL(updateCatheterView()),  dsp, SLOT(updateCatheterView()) );

//    connect( dsp, SIGNAL( sendWarning( QString ) ), this, SIGNAL( sendWarning( QString ) ) );
//    connect( dsp, SIGNAL( sendError( QString ) ),   this, SIGNAL( sendError( QString ) ) );

//#if ENABLE_IPP_FFT_TUNING
//    connect( this, SIGNAL(magScaleValueChanged(int)), dsp, SLOT(setMagScaleFactor(int) ) );
//    connect( this, SIGNAL(fftScaleValueChanged(int)), dsp, SLOT(setFftScaleFactor(int) ) );
//#endif

//#if ENABLE_LOW_SPEED_DATA_SNAPSHOT
//    connect( this, SIGNAL(saveSignals()), dsp, SLOT(saveSignals()) );
//#endif

//#if CONSOLE_MANUFACTURING_RELEASE
//    connect( this, SIGNAL(enableOcelotSwEncoder(bool)), dsp, SLOT(enableOcelotSwEncoder(bool)) );
//#endif

//    // configure the hardware
//    if( !configure() )
//    {
//        // fatal error
//        emit sendError( tr( "DAQ hardware could not be initialized." ) );
//    }

//    // initialize the DSP
//    dsp->init( bd.RecLength, linesPerRevolution, bytesPerRecord, bytesPerBuffer, channelCount );

//    deviceSettings &dev = deviceSettings::Instance();
//    encoderCounts = dev.current()->getLinesPerRevolution();

//    emit attenuateLaser( false ); // enable laser and EVOA controls at start of low speed DAQ

//    qDebug() << "LowSpeedDAQ::init end";
//}

///*
// * configure
// *
// * Set up the AlazarTech DAQ hardware.  Most settings are hard-coded for this
// * application; settings that are configurable are loaded from the configuration
// * file in the Avinger System directory.
// *
// * The shared memory for the data producer (this) and consumer (frontend) threads
// * is also allocated here.
// *
// * Any errors in setting up the hardware will case this function to fail and
// * return an error to the caller.

// */
//bool LowSpeedDAQ::configure( void )
//{
//    RETURN_CODE status = ApiSuccess;

//    LOG( INFO, "Low Speed DAQ start-up" );

//    // Load the settings specific to this catheter
//    deviceSettings &devSettings = deviceSettings::Instance();
//    linesPerRevolution = devSettings.current()->getLinesPerRevolution();

//    // Load the settings for the DAQ specific to this console
//    DaqSettings &settings = DaqSettings::Instance();

//    bd.RecordCount = settings.getRecordCount();    // Specify how many records to capture
//    bd.RecLength   = settings.getRecordLength();   // Specify Length of each record in number of samples
//    bd.PreDepth    = settings.getPreDepth();       // Number of samples to use before the trigger
//    bd.ClockSource = INTERNAL_CLOCK;
//    bd.ClockEdge   = CLOCK_EDGE_RISING;
//    bd.SampleRate  = SAMPLE_RATE_100MSPS;          // Specify the sample rate at which to capture data

//    // low speed setup
//    // ===============
//    bytesPerRecord = 2 * bd.RecLength;
//    bytesPerBuffer = bytesPerRecord * 1;
//    channelCount = 1;
//    // ===============

//    // Channel A: OCT signal
//    bd.CouplingChanA   = DC_COUPLING;
//    bd.InputRangeChanA = settings.getChAInputRange();
//    bd.InputImpedChanA = IMPEDANCE_50_OHM;

//    // Channel B: encoder signal
//    bd.CouplingChanB   = DC_COUPLING;
//    bd.InputRangeChanB = settings.getChBInputRange();
//    bd.InputImpedChanB = IMPEDANCE_1M_OHM;

//    bd.TriEngOperation = TRIG_ENGINE_OP_J;

//    bd.TriggerEngine1  = TRIG_ENGINE_J;
//    bd.TrigEngSource1  = TRIG_EXTERNAL;
//    bd.TrigEngSlope1   = TRIGGER_SLOPE_NEGATIVE;
//    bd.TrigEngLevel1   = 160;

//    // Trigger engine 2 is not used by this application (see bd.TriEngOperation
//    // above). The configuration here is for completeness and so we know the
//    // hardware is in a known state.
//    bd.TriggerEngine2  = TRIG_ENGINE_K;
//    bd.TrigEngSource2  = TRIG_DISABLE;
//    bd.TrigEngSlope2   = TRIGGER_SLOPE_POSITIVE;
//    bd.TrigEngLevel2   = 128;

//    /*
//     * Allocate the data buffer in which the device will DMA transfer the
//     * captured data.
//     *
//     * NOTE:
//     * Each data buffer must be a minimum of 16 samples larger
//     * than the Record Length setting of the board.
//     *
//     * For example, if your Record Length is 1024 samples, your buffer
//     * must be at least 1040 samples long.
//     *
//     * If you do not allocate this additional space, your application
//     * may crash when you issue an AlazarRead command to DMA data from
//     * the digitizer to host memory,
//     */
//    const U32 DMAPadding = 16;
//    m_daqRawData[ ChannelA ] = (U16 *)malloc( ( bd.RecLength + DMAPadding ) * sizeof( U16 ) );
//    if( m_daqRawData[ ChannelA ] == NULL )
//    {
//        CHECK_ERROR;
//    }

//    m_daqRawData[ ChannelB ] = (U16 *)malloc( ( bd.RecLength + DMAPadding ) * sizeof( U16 ) );
//    if( m_daqRawData[ ChannelB ] == NULL )
//    {
//        CHECK_ERROR;
//    }
//#if QT_NO_DEBUG
//    // Set-up memory for the raw data in each element of the shared memory
//    TheGlobals::instance()->allocateFrameData();
//#endif
//    // Assume 1 board only in the system
//    hBoard = AlazarGetBoardBySystemID( 1, 1 );
//    AlazarSetLED( hBoard, 0 );

//    status = AlazarSetRecordCount( hBoard, bd.RecordCount );
//    CHECK_ERROR;

//    status = AlazarSetRecordSize( hBoard, bd.PreDepth, bd.RecLength - bd.PreDepth );
//    CHECK_ERROR;

//    status = AlazarSetCaptureClock( hBoard,
//                                    bd.ClockSource,
//                                    bd.SampleRate,
//                                    bd.ClockEdge,
//                                    0 );
//    CHECK_ERROR;

//    status = AlazarInputControl( hBoard,
//                                 CHANNEL_A,
//                                 bd.CouplingChanA,
//                                 bd.InputRangeChanA,
//                                 bd.InputImpedChanA );
//    CHECK_ERROR;

//    status = AlazarInputControl( hBoard,
//                                 CHANNEL_B,
//                                 bd.CouplingChanB,
//                                 bd.InputRangeChanB,
//                                 bd.InputImpedChanB );
//    CHECK_ERROR;

//    status = AlazarSetTriggerOperation( hBoard,
//                                        bd.TriEngOperation,
//                                        bd.TriggerEngine1,
//                                        bd.TrigEngSource1,
//                                        bd.TrigEngSlope1,
//                                        bd.TrigEngLevel1,
//                                        bd.TriggerEngine2,
//                                        bd.TrigEngSource2,
//                                        bd.TrigEngSlope2,
//                                        bd.TrigEngLevel2 );
//    CHECK_ERROR;

//    if( ( bd.TrigEngSource1 == TRIG_EXTERNAL ) ||
//            ( bd.TrigEngSource2 == TRIG_EXTERNAL ) )
//    {
//        AlazarSetExternalTrigger( hBoard, DC_COUPLING, ETR_DIV5 ); // Set External Trigger to be 5V range, DC coupling
//    }

//    // Create a 1 Second timeout delay
//    AlazarSetTriggerTimeOut( hBoard, 100000 );

//    // Make sure top-dead-center detection is turned off
//    status = AlazarConfigureAuxIO( hBoard, AUX_INPUT_AUXILIARY, TRIGGER_SLOPE_POSITIVE );
//    qDebug() << "set aux.  Status = " << AlazarErrorToText( status );

//    isConfigured = true;

//    return true;
//}


///*
// * getData - Low Speed
// *
// * Interface to the DAQ hardware to start the capture and transfer data from the board memory
// * to the PC memory.  We are using the AlazarTech Traditional API.
// */
//bool LowSpeedDAQ::getData( void )
//{
//    TIME_THIS_SCOPE( getData );

//    {
//        TIME_THIS_SCOPE( StartCapture );
//        AlazarStartCapture( hBoard );
//        while( AlazarBusy( hBoard ) ) /* wait for trigger */;
//    }
//    {
//        TIME_THIS_SCOPE( ReadData );

//        // Read the acquired data from the device -- always fill in the first two raw data buffers
//        AlazarRead( hBoard, CHANNEL_A, m_daqRawData[ ChannelA ], sizeof( unsigned short ), recordToRead, -(long)bd.PreDepth, bd.RecLength );
//        AlazarRead( hBoard, CHANNEL_B, m_daqRawData[ ChannelB ], sizeof( unsigned short ), recordToRead, -(long)bd.PreDepth, bd.RecLength );
//    }

//    TheGlobals::instance()->incrementGDaqRawDataCompleted();

//    // index into the shared data for DAQ and DSP
//    TheGlobals::instance()->updateGDaqRawData_idx();

//    return true;
//}
