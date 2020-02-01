/*
 * highspeeddaq.cpp
 *
 * Implements the DAQ interface for high speed devices. High speed devices
 * acquire and process an entire image worth of A-lines at a time and
 * provide the fully baked image to the frontend.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 *
 */
#include "highspeeddaq.h"
#include "deviceSettings.h"
#include "logger.h"
#include "buildflags.h"
#include "profiler.h"
#include "dspgpu.h"
#include "Utility/userSettings.h"
#include "AlazarCmd.h"
#include "AlazarApiFunction.h"
#include "daqSettings.h"
#include "util.h"
#include "ipp.h"
#include "playbackmanager.h"
#include "theglobals.h"


/*
 * Timeout before filling a full frame buffer. This adds to the time spent waiting
 * during device select if a Top Dead Center trigger isn't sent (sled turned off).
 * This value should be greater than 2 x the time it takes to recieve one frame.
 */
const int ReceiveBufferTimeout_ms = 250;

/*
 * Constructor
 */
HighSpeedDAQ::HighSpeedDAQ( void )
{
    buffersPosted = sizeof( captureBuffer ) / sizeof( U16 * );
    qDebug() << "BuffersPosted: " <<  buffersPosted;
    LOG1(buffersPosted);
}

/*
 * Destructor
 *
 * Free memory
 */
HighSpeedDAQ::~HighSpeedDAQ()
{
    for( int kr = 0; kr < buffersPosted; kr++ )
    {
        if( captureBuffer[ kr ] != NULL )
        {
            free( captureBuffer[ kr ] );
            captureBuffer[ kr ] = NULL;
        }
    }

#if SYNTHDATA
    for( int i = 0; i < SYNTHETIC_DATA_BUFFERS; i++ )
    {
        if( syntheticData[ i ] != NULL )
        {
            free( syntheticData[ i ] );
            syntheticData[ i ] = NULL;
        }
    }
#endif
}

/*
 * init
 *
 * Initialize the components of the DAQ necessary for low speed devices.
 * The GPU DSP is created and initialized here
 *
 */
void HighSpeedDAQ::init( void )
{
    qDebug() << "HighSpeedDAQ::init start";

    // Call the common DAQ initialization
    DAQ::init();

    // Create the DSP
    dsp = new DSPGPU();

    if( dsp == NULL )
    {
        // fatal error
        emit sendError( tr( "DSP thread could not be created." ) );
    }

    // Pass messages to/from the DSP
    connect( this, SIGNAL(setBlackLevel(int)),              dsp,  SLOT(setBlackLevel(int)) );
    connect( this, SIGNAL(setWhiteLevel(int)),              dsp,  SLOT(setWhiteLevel(int)) );
    connect( this, SIGNAL(setAveraging(bool)),              dsp,  SLOT(setAveraging(bool)) );
    connect( this, SIGNAL(handleDisplayAngle(float)),       dsp,  SLOT(setDisplayAngle(float)) );
    connect( this, SIGNAL(setInvertColors(bool)),           dsp,  SLOT(setInvertColors(bool)) );
    connect( this, SIGNAL(setFrameAverageWeights(int,int)), dsp,  SLOT(setFrameAverageWeights(int,int)) );
    connect( this, SIGNAL(updateCatheterView()),            dsp,  SLOT(updateCatheterView()) );
    connect( dsp,  SIGNAL( sendWarning( QString ) ),        this, SIGNAL( sendWarning( QString ) ) );
    connect( dsp,  SIGNAL( sendError( QString ) ),          this, SIGNAL( sendError( QString ) ) );


#if ENABLE_RAW_DATA_SNAPSHOT
    connect( this, SIGNAL(rawDataSnapshot( int )), dsp, SLOT(recordRawData( int )) );
#endif

    qDebug() << "High Speed DAQ init start";

    // configure the hardware
    if( !configure() )
    {
        // fatal error
        emit sendError( tr( "DAQ hardware could not be initialized." ) );
    }

    // initialize the DSP thread
    dsp->init( bd.RecLength, linesPerRevolution, bytesPerRecord, bytesPerBuffer, channelCount );

    timeoutCounter = 0;
    emit attenuateLaser( false );

    qDebug() << "HighSpeedDAQ::init end";
}


/*
 * configure
 *
 * Set up the AlazarTech DAQ hardware.  Most settings are hard-coded for this
 * application; settings that are configurable are loaded from the configuration
 * file in the Avinger System directory.
 *
 * The shared memory for the data producer (this) and consumer (frontend) threads
 * is also allocated here.
 *
 * Any errors in setting up the hardware will case this function to fail and
 * return an error to the caller.
 */
bool HighSpeedDAQ::configure( void )
{
    deviceSettings &devSettings = deviceSettings::Instance();
    linesPerRevolution = devSettings.current()->getLinesPerRevolution();

    qDebug() << "DAQ: Number lines per revolution:" << linesPerRevolution;

    DaqSettings &settings = DaqSettings::Instance();

    // Select which channels to capture (A, B, or both)
    // Channel A only for for unidirectional rotation
    U32 channelMask = CHANNEL_A; //| CHANNEL_B;

    // Calculate the number of enabled channels from the channel mask
    switch( channelMask )
    {
    case CHANNEL_A: // intentional fall-through
    case CHANNEL_B:
        channelCount = 1;
        qDebug() << "DAQ: Reading 1 channel only";
        break;
    case CHANNEL_A | CHANNEL_B:
        channelCount = 2;
        qDebug() << "DAQ: Reading 2 channels";
        break;
    default:
        qDebug() << "Error: Invalid channel mask " << channelMask;
        return false;
    }

    // Get the sample size in bits, and the on-board memory size in samples per channel

    // Calculate the size of each DMA buffer in bytes
    // Get the sample size in bits, and the on-board memory size in samples per channel
    // set the expected values from the hardware for debug build
    U8  bitsPerSample        = 16;
    U32 maxSamplesPerChannel = ( 64 /*MB*/ * KB_per_MB ) * B_per_KB;

    RETURN_CODE status = ApiSuccess;

    status = AlazarGetChannelInfo( hBoard, &maxSamplesPerChannel, &bitsPerSample );
    qDebug() << "get channel info = " << AlazarErrorToText( status );
    CHECK_ERROR;

    // Select the number of pre-trigger samples per record
    U32 preTriggerSamples = settings.getPreDepth();

    // Select the number of post-trigger samples per record
    U32 postTriggerSamples = settings.getRecordLength() - preTriggerSamples;

    // Specify the number of records per DMA buffer
    recordsPerBuffer = linesPerRevolution;

    // Calculate the size of each DMA buffer in bytes
    U32 bytesPerSample   = ( bitsPerSample + 7 ) / 8;
    U32 samplesPerRecord = preTriggerSamples + postTriggerSamples;
    bytesPerRecord       = bytesPerSample * samplesPerRecord;
    bytesPerBuffer       = bytesPerRecord * recordsPerBuffer * channelCount;

    qDebug() << "bits per sample      = " << bitsPerSample;
    qDebug() << "maxSamplesPerChannel = " << maxSamplesPerChannel;
    qDebug() << "preTrigger count     = " << preTriggerSamples;
    qDebug() << "postTrigger count    = " << postTriggerSamples;
    qDebug() << "record length        = " << settings.getRecordLength();
    qDebug() << "channelCount         = " << channelCount;
    qDebug() << "buffersPosted        = " << buffersPosted;
    qDebug() << "bytesPerSample       = " << bytesPerSample;
    qDebug() << "samplesPerRecord     = " << samplesPerRecord;
    qDebug() << "bytesPerRecord       = " << bytesPerRecord;
    qDebug() << "recordsPerBuffer     = " << recordsPerBuffer;
    qDebug() << "bytesPerBuffer       = " << bytesPerBuffer << "B, " <<  bytesPerBuffer/1024.0 << "kB," << bytesPerBuffer/1024.0/1024.0 << "MB";

    buffersCompleted   = 0;

    // *****************************
    LOG( INFO, "High Speed DAQ start-up" );

    bd.RecordCount     = recordsPerBuffer; // settings.getRecordCount();    // Specify how many records to capture
    bd.RecLength       = postTriggerSamples + preTriggerSamples; //settings.getRecordLength();   // Specify Length of each record in number of samples
    bd.PreDepth        = preTriggerSamples; //settings.getPreDepth();       // Number of samples to use before the trigger
    bd.ClockSource     = INTERNAL_CLOCK;
    bd.ClockEdge       = CLOCK_EDGE_RISING;
    bd.SampleRate      = SAMPLE_RATE_100MSPS;          // Specify the sample rate at which to capture data

    // Channel A: OCT signal
    bd.CouplingChanA   = DC_COUPLING;
    bd.InputRangeChanA = settings.getChAInputRange();
    bd.InputImpedChanA = IMPEDANCE_50_OHM;

    // Channel B: not used for high speed.  Set to known values.
    bd.CouplingChanB   = DC_COUPLING;
    bd.InputRangeChanB = settings.getChBInputRange();
    bd.InputImpedChanB = IMPEDANCE_1M_OHM;

    bd.TriEngOperation = TRIG_ENGINE_OP_J;

    bd.TriggerEngine1  = TRIG_ENGINE_J;
    bd.TrigEngSource1  = TRIG_EXTERNAL;
    bd.TrigEngSlope1   = TRIGGER_SLOPE_NEGATIVE;
    bd.TrigEngLevel1   = 160;

    // Trigger engine 2 is not used by this application (see bd.TriEngOperation
    // above). The configuration here is for completeness and so we know the
    // hardware is in a known state.
    bd.TriggerEngine2  = TRIG_ENGINE_K;
    bd.TrigEngSource2  = TRIG_DISABLE;
    bd.TrigEngSlope2   = TRIGGER_SLOPE_NEGATIVE;
    bd.TrigEngLevel2   = 128;

    /*
     * Allocate the data buffers in which the device will DMA transfer the
     * captured data.
     */
    qDebug() << "Allocating raw data buffers";
//lcv - moved to the TheGlobal constructor
// #if QT_NO_DEBUG
//    TheGlobals::instance()->allocateDaqRawDataBuffer(bytesPerBuffer);

//    // Set-up memory for the raw data in each element of the shared memory
//    TheGlobals::instance()->allocateFrameData();
//#endif
    /*
     * ASync buffer setup
     */
    qDebug() << "bd.RecLength: " << bd.RecLength;
    qDebug() << "Allocating capture buffers (bytesPerBuffer): " << bytesPerBuffer << "B";

    // allocate space for the DAQ to DMA data into
    for( int kr = 0; kr < buffersPosted; kr++ )
    {
        captureBuffer[ kr ] = static_cast<U16 *>(malloc( bytesPerBuffer ));
        if( captureBuffer[ kr ] == nullptr )
        {
            // If we ran out of memory while allocating buffers, log how far we got
            LOG( FATAL, QString( "Could not allocate memory for the capture buffers! Capture buffer: " ).arg ( kr ) );

            // fatal error
            emit sendError( tr( "Could not allocate memory for the capture buffers." ) );
        }
    }

    // Assume 1 board only in the system
    hBoard = AlazarGetBoardBySystemID( 1, 1 );
    AlazarSetLED( hBoard, 0 );
    qDebug() << "ATS board handle = " << hBoard;

    status = AlazarSetRecordCount( hBoard, bd.RecordCount );
    qDebug() << "set record status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    status = AlazarSetRecordSize( hBoard, bd.PreDepth, bd.RecLength - bd.PreDepth );
    qDebug() << "set record size status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    status = AlazarSetCaptureClock( hBoard,
                                    bd.ClockSource,
                                    bd.SampleRate,
                                    bd.ClockEdge,
                                    0 );
    qDebug() << "set capture clock status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    status = AlazarInputControl( hBoard,
                                 CHANNEL_A,
                                 bd.CouplingChanA,
                                 bd.InputRangeChanA,
                                 bd.InputImpedChanA );
    qDebug() << "set input control A status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    status = AlazarSetBWLimit( hBoard,
                               CHANNEL_A,
                               0 );  // disable
    qDebug() << "set input BW Limit A status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    status = AlazarInputControl( hBoard,
                                 CHANNEL_B,
                                 bd.CouplingChanB,
                                 bd.InputRangeChanB,
                                 bd.InputImpedChanB );
    qDebug() << "set input control B status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    status = AlazarSetBWLimit( hBoard,
                               CHANNEL_B,
                               0 );  // disable
    qDebug() << "set input BW Limit B status = " << AlazarErrorToText( status );
    CHECK_ERROR;


    status = AlazarSetTriggerOperation( hBoard,
                                        bd.TriEngOperation,
                                        bd.TriggerEngine1,
                                        bd.TrigEngSource1,
                                        bd.TrigEngSlope1,
                                        bd.TrigEngLevel1,
                                        bd.TriggerEngine2,
                                        bd.TrigEngSource2,
                                        bd.TrigEngSlope2,
                                        bd.TrigEngLevel2 );
    qDebug() << "set trigger status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    if( ( bd.TrigEngSource1 == TRIG_EXTERNAL ) ||
        ( bd.TrigEngSource2 == TRIG_EXTERNAL ) )
    {
        AlazarSetExternalTrigger( hBoard, DC_COUPLING, ETR_DIV5 ); // Set External Trigger to be 5V range, DC coupling
    }

    status = AlazarSetTriggerDelay( hBoard, 0 );
    qDebug() << "set trigger delay status = " << AlazarErrorToText( status );
    CHECK_ERROR;

    // Create a 1 Second timeout delay
    status = AlazarSetTriggerTimeOut( hBoard, 100000 );
    qDebug() << "set trigger timeout status = " << AlazarErrorToText( status );
    CHECK_ERROR;

   /*
    * Use the AUX connector as a frame trigger / top-dead center (TDC) mark -- requires API and DLL ver 6.0.2 or higher
    */
    status = AlazarConfigureAuxIO( hBoard, AUX_IN_TRIGGER_ENABLE, TRIGGER_SLOPE_POSITIVE );
    qDebug() << "set aux.  Status = " << AlazarErrorToText( status );

    /*
     * Async setup
     */
    U32 CFLAGS = ADMA_TRADITIONAL_MODE |
                 ADMA_EXTERNAL_STARTCAPTURE;

    status = AlazarBeforeAsyncRead( hBoard,
                                    channelMask,
                                    -(long)bd.PreDepth,
                                    bd.RecLength,
                                    recordsPerBuffer,
                                    0x7fffffff,        // acquire until told to stop
                                    CFLAGS );

    qDebug() << "AlazarBeforeAsyncRead status = " << AlazarErrorToText( status );

    if( status != ApiSuccess )
    {
        AlazarAbortAsyncRead( hBoard );// abort any ongoing async reads if the card was left in a weird state

        // fatal error
        qDebug() << "Error calling AlazarBeforeAsyncRead() status =" << AlazarErrorToText( status );
        emit sendError( QString( "Error calling AlazarBeforeAsyncRead(): %1" ).arg( QString( AlazarErrorToText( status ) ) ) );
    }

    /*
     * Post the buffers to the controller
     */
    for( int kr = 0; kr < buffersPosted; kr++ )
    {
        status = AlazarPostAsyncBuffer( hBoard, captureBuffer[ kr ], bytesPerBuffer );

        if( status != ApiSuccess )
        {
            AlazarAbortAsyncRead( hBoard );// abort any ongoing async reads if the card was left in a weird state

            // fatal error
            qDebug() << "Error posting async buffer" << kr << "status =" << AlazarErrorToText( status );
            emit sendError( QString( "Error posting async buffers: %1" ).arg( QString( AlazarErrorToText( status ) ) ) );
        }
    }
    qDebug() << "AlazarPostAsyncBuffer(): BuffersPosted = " << buffersPosted;

    buffersCompleted = 0;
    bufferIndex = 0;

    isConfigured = true;

    return true;
}

/*
 * getData
 *
 * Interface to the DAQ hardware to start the capture and transfer data from the board memory
 * to the PC memory.  We are using the AlazarTech Asynchronous AutoDMA API.
 */
bool HighSpeedDAQ::getData( void )
{
    TIME_THIS_SCOPE( getData );
    bool gotData = false;
    static bool notifyApiWaitTimeout = true;

    /*
     * Async DMA
     */
    RETURN_CODE status = ApiSuccess;
    static U16 *pBuffer;

    bufferIndex = buffersCompleted % buffersPosted;

    //lcv use the buffer from the Playback manager
    pBuffer = captureBuffer[ bufferIndex ];

    status = AlazarWaitAsyncBufferComplete( hBoard, (void *)pBuffer, ReceiveBufferTimeout_ms );

    if( status == ApiWaitTimeout )
    {
        // Only write to debug on the first Timeout, instead of flooding.
        if( notifyApiWaitTimeout )
        {
            notifyApiWaitTimeout = false;
            qDebug() << "ApiWaitTimeout";
        }

        timeoutCounter++;
        if( timeoutCounter == 10 )
        {
            emit attenuateLaser( true );
        }
        return gotData;
    }

    // repost this buffer only if AlazarWaitAsyncBufferComplete() was successful
    if( status == ApiSuccess )
    {
        TheGlobals::instance()->updateRawDataIndex();

        // copy to rawData buffers
        const U16 *pA = TheGlobals::instance()->getRawDataBufferPointer();

        if(PlaybackManager::instance()->isPlayback()){
            PlaybackManager::instance()->retrieveRawData(TheGlobals::instance()->getRawDataIndex(), pA);
            buffersCompleted++;
            TheGlobals::instance()->incrementRawDataIndexCompleted();
        } else {
            // Copy from the DMA buffer to our internal structure  -- would be nice not to do this
            ippsCopy_16s( (Ipp16s *)pBuffer, (Ipp16s *)pA, bytesPerBuffer / 2  );

            buffersCompleted++;
            TheGlobals::instance()->incrementRawDataIndexCompleted();
            PlaybackManager::instance()->recordRawData(TheGlobals::instance()->getRawDataIndex(), TheGlobals::instance()->getRawDataIndexCompleted());
            // Repost the buffer so the DAQ can use it again
        }
        status = AlazarPostAsyncBuffer( hBoard, pBuffer, bytesPerBuffer );

        gotData = true;
        notifyApiWaitTimeout = true;

        if( timeoutCounter != 0 )
        {
            timeoutCounter = 0;
            emit attenuateLaser( false );
        }

    }

    if( ( status != ApiSuccess ) && ( status != ApiTransferComplete ) )
    {
        if( status != ApiDmaInProgress )
        {
            /*
             * DAQ is probably hung up and needs to be restarted. Emit the signal to frontend
             * to restart the DAQ.
             */
            stop();
            emit signalDaqResetToFrontend();
            LOG( WARNING, "HighSpeedDAQ acquisition failure detected! Possibly caused by monitor connect or disconnect with the Pantheris Sled turned on." );

            static int errCount = 0;
            if( ++errCount == 10000 ) // slow down error stream during testing
            {
                qDebug() << "Repost failed! status = " << AlazarErrorToText( status );
                errCount = 0;
            }
        }
    }

    return gotData;
}
