/*
 * SoftwareDSP.cpp
 *
 * Interface for the CPU-based digital signal processing functions.
 *
  * Author(s): Dennis W. Jackson, Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include "softwaredsp.h"
#include "defaults.h"
#include "logger.h"
#include "profiler.h"
#include "daqSettings.h"
#include "deviceSettings.h"
#include "util.h"
#include "buildflags.h"
#include "theglobals.h"

// Raw data from the DAQ hardware. Each element points to the start of Channel data

// FFT length is N = 2^order
#define FFT_ORDER (11)
#define FFT_NORM_METHOD (IPP_FFT_DIV_INV_BY_N)
#define FFT_HINT (ippAlgHintAccurate)

#define LOG_TABLE_SIZE (65536)
#define LOG_SCALE_FACTOR ( ( LOG_TABLE_SIZE - 1 ) / (10 * log10( double( LOG_TABLE_SIZE - 1 ) ) ) )

/*
 * constructor
 */
SoftwareDSP::SoftwareDSP()
{
    qDebug() << "SoftwareDSP constructor";

    // Default FFT config
    fftData.fftSpec    = nullptr;
    fftData.fftChABuf  = nullptr;
    fftData.fftWorkBuf = nullptr;
    fftData.fftOrder   = FFT_ORDER;
    fftData.inputSize  = (1 << FFT_ORDER);
    fftData.fftPoints  = (1 << FFT_ORDER);

    /*
     * These parameters affect the FFT routine and need careful tuning to keep
     * within the full dynamic range without saturating. This is empirically
     * determined with a function generator producing a single sin waveform at
     * 95% of full ADC range and adjusting until saturation no longer occurs.
     *
     * Sin: 1 MHz, 400 mV p-p, 200 mV offset
     */
    fftData.magScaleFactor = 1;
    fftData.fftScaleFactor = 5;

    // log lookup table
    logTable = nullptr;

    // save the default settings in case the device is changed during the session
    DaqSettings       &settings = DaqSettings::Instance();
    deviceSettings &devSettings = deviceSettings::Instance();

    defaultEncoderSettings.calMinVal    = settings.getEncoderCalMin();
    defaultEncoderSettings.calMaxVal    = settings.getEncoderCalMax();
    defaultEncoderSettings.gearingRatio = 1;

    // XXX: Not a big fan of this pattern
    if( devSettings.current() )
    {
        defaultEncoderSettings.linesPerRev = U16(devSettings.current()->getLinesPerRevolution());
    }
    else
    {
        // Default to a 1024 count encoder
        defaultEncoderSettings.linesPerRev = 1024;
    }

    // Set the default encoder values
    setEncoderConfigurationToDefault();

    // encoder variables
    currAngle.rawAngle_cnt = 0;
    currAngle.offset_cnt   = 0;
    currAngle.angle_cnt    = 0;
    prevAngle              = currAngle;

    // Allow changes in the device to reset the configuration
    connect( &devSettings, SIGNAL( deviceChanged() ), this, SLOT( setEncoderConfigurationToDefault() ) ); //XXX: need now that DAQ and DSP are deleted between dev changes? any other path to this?

    // measure times to compute the encoder rotation rate
    encoderTimer.start();
    prevTime = 0;
    currTime = 0;

    resampledData = nullptr;
}

/*
 * Destructor
 */
SoftwareDSP::~SoftwareDSP()
{
    qDebug() << "SoftwareDSP::~SoftwareDSP()";
    LOG( INFO, "SoftwareDSP shutdown" )

//    if( adjustBuffer )
//    {
//        delete [] adjustBuffer;
//    }

//    if( resampledData != NULL )
//    {
//        delete [] resampledData;
//        resampledData = NULL;
//    }

//    if( fftData.fftChABuf )
//    {
//        delete [] fftData.fftChABuf;
//    }

//    if( fftData.fftWorkBuf )
//    {
//        delete [] fftData.fftWorkBuf;
//    }

//    if( logTable )
//    {
//        delete [] logTable;
//    }

//    ippsFFTFree_R_16s( fftData.fftSpec );
}

/*
 * init
 *
 * Initialize memory and compute look up tables.
 */
void SoftwareDSP::init( unsigned int inputLength,
                        unsigned int frameLines,
                        unsigned int inBytesPerRecord,
                        unsigned int inBytesPerBuffer,
                        int inChannelCount )
{
    // call the common initilization steps
    DSP::init( inputLength, frameLines, inBytesPerRecord, inBytesPerBuffer, inChannelCount );

    // Configure the DSP
    checkInitFFTStructs();
    computeLogTable();

    // Memory for working on the FFT data
    adjustBuffer = new unsigned short[ fftData.fftPoints ];

    if( adjustBuffer )
    {
        // fatal error
        displayFailureMessage( tr( "Could not allocate memory for DSP display buffers" ), true );
    }

    resampledData = new unsigned short[ DSP::RescalingDataLength ];

    if( resampledData )
    {
        emit sendError( tr( "Error allocating space in DSP::init" ) );
    }
}

/*
 * processData
 *
 * Do all the work we need to do to an A-line before handing it off to be displayed
 *    - Shift from AlazarTech's MSB storage to a standard format
 *    - Rescale the data according to the laser characterization data for fine-tuning the waveform
 *    - Transform the data (FFT, magnitude, log)
 *    - Compute the encoder angle
 *    - Bundle all of the data up for displaying and storing
 *
 * Data is shared across threads via a global data buffer.  A single counter indexes
 * this buffer.  The DAQ writes to a slot in the buffer while the consumer alwayes
 * reads one slot behing the current index.  This buffer is lossy on purpose; the
 * laser produces more lines than we can transfer and display; there is no loss of
 * clinically relevant data since many of the lines are of the same view.
 */
void SoftwareDSP::processData( void )
{
//    TIME_THIS_SCOPE( dsp_processData );

//    // initialize prevIndex to the same value that index will get so no
//    // work is done until the DAQ starts up
//    static int prevIndex = TheGlobals::instance()->getPrevGDaqRawData_idx();

//    // which index to point into for the raw data
//    int index = TheGlobals::instance()->getPrevGDaqRawData_idx();

//    // Only process data if there it had been updated
//    if( index != prevIndex )
//    {
//        TIME_THIS_SCOPE( SoftwareDSP_processData );

//        // determine where to store this frame of data and store everything we need
//        pData = TheGlobals::instance()->getFrameDataPointer();

//#if ENABLE_FRAME_COUNTERS_TO_DEBUG
//        // Rough lines/second counter  XXX
//        static int frameCount = 0;
//        frameCount++;
//        if( frameTimer.elapsed() > 1000 )
//        {
//            qDebug() << "                       DSP frameCount/s:" << frameCount;
//            frameCount = 0;
//            frameTimer.restart();
//        }
//#endif

//        // rescale using the laser calibration constants
//        rescale( m_daqRawData[ 0 ] ); //output is in resampledData

//        // Copy the resampled raw data into the shared memory buffer. The buffer
//        // resampledData is used for in-place transforming of the data so it must be
//        // copied before any other processing takes place.
//        ippsCopy_16s( (Ipp16s *)m_daqRawData[ 0 ], (Ipp16s *)pData->rawData, bytesPerRecord / 2 );

//        // FFT, magnitude, log
//        if( !transformData_sw( resampledData ) )
//        {
//            emit sendError( tr( "DSP Data transformation failure." ) );
//        }

//        // XXX: use 14-bit values since we are shifting the encoder values in SoftwareDSP.  Using the full range
//        //      caused integer overflow in the encoder calculations. See ticket #1148.
//        ippsRShiftC_16u_I( 2, m_daqRawData[ 1 ], recordLength );

//        // Copy frame data into the shared memory buffer
//        if( useDistalToProximalView )
//        {
//            // use the encoder data directly
//            pData->encoderPosition = computeEncoderAngle( m_daqRawData[ 1 ], recordLength );
//        }
//        else
//        {   // proximal to distal view
//            //    Subtract from the max encoder value ( encoderCounts - 1 ) to draw in the
//            //    reverse direction (i.e., clockwise rotation of the encoder
//            //    draws counterclockwise on the screen).
//            pData->encoderPosition = ( encoderSettings.linesPerRev - 1 ) - computeEncoderAngle( m_daqRawData[ 1 ], recordLength );
//        }
//        pData->frameCount      = 0;  // Frame counts are filled in by the Data consumer.
//                                     // This keeps the frame counts continuous.
//        pData->timeStamp       = getTimeStamp();
//        pData->milliseconds    = getMilliseconds();


//        // Copy the processed data into the shared memory buffer
//        ippsCopy_16s( (Ipp16s *)resampledData, (Ipp16s *)pData->fftData, FFTDataSize );

//        // Convert the A-line to a display-friendly values
//        if( !adjustForDisplay( pData->fftData, pData->dispData, FFTDataSize / 2 ) )
//        {
//            emit sendError( tr( "DSP::adjustForDisplay() failed" ) );
//        }


//#if CONSOLE_MANUFACTURING_RELEASE
//        /*
//         * Generate a synthetic encoder signal to draw the sector sweep.
//         */
//        if( useSwEncoder )
//        {
//            static unsigned int encoderStep = 0;
//            pData->encoderPosition = encoderStep / 4; // Divide by 4 to slow down the encoder rate.
//            encoderStep++;
//            if( ( encoderStep / 4 ) > ( encoderSettings.linesPerRev - 1 ) ) // Clamp values to linesPerRev.
//            {
//                encoderStep = 0;
//            }
//        }
//#endif

//#if TEST_SPIDERWEB
//        /*
//     * Overwrite all of the FFT data and display data and replace it with concentric rings and radial lines
//     */
//        int testAng = pData->encoderPosition;

//        for( unsigned int kr = 0; kr < ALineLengthNormal_px; kr++ )
//        {
//            pData->fftData[ kr ] = 0;
//            pData->dispData[ kr ] = 0;

//            if( ( kr == 100 ) || ( kr == 101 ) ||
//                ( kr == 200 ) || ( kr == 201 ) ||
//                ( kr == 300 ) || ( kr == 301 ) ||
//                ( kr == 400 ) || ( kr == 401 ) ||
//                ( kr == 500 ) || ( kr == 501 ) ||
//                ( kr == 510 ) || ( kr == 511 ) )
//            {
//                pData->fftData[ kr ] = 60000;
//                pData->dispData[ kr ] = 255;
//            }

//            // Mod 128 = 8 radial lines
//            if( ( ( testAng       % 128 ) == 0 ) ||
//                ( ( testAng - 1 ) % 128 ) == 0 )
//            {
//                // white lines. Update the fftData so recordings see this as well.
//                pData->fftData[ kr ]  = 60000;
//                pData->dispData[ kr ] = 255;
//            }
//        }
//#endif

//        // Update the global index into the shared buffer. This must be the last thing in the thread
//        // to make sure that the consumer thread only sees completely filled data structures.
//        TheGlobals::instance()->inrementGFrameCounter();
//        prevIndex = index;
//    }
}


/*
 * checkInitFFTStructs
 *
 * Set up and check the data structures for using IPP
 */
void SoftwareDSP::checkInitFFTStructs( void )
{
//    // IPP-defined constant
//    const int CCSFormatOverhead( 2 );

//    // Allocate the fft buffers
//    int fftBufSize = (1 << fftData.fftOrder) + CCSFormatOverhead;

//    fftData.fftChABuf = new unsigned short[ fftBufSize ];

//    if( !fftData.fftChABuf )
//    {
//        // fatal error
//        emit sendError( tr( "Failed to allocate buffers for FFT." ) );
//    }

//    // Set up IPP's fft spec structure
//    IppStatus retval = ippsFFTInitAlloc_R_16s( &fftData.fftSpec,
//                                               fftData.fftOrder,
//                                               FFT_NORM_METHOD,
//                                               FFT_HINT );

//    if( retval != ippStsNoErr )
//    {
//        // fatal error
//        emit sendError( tr( "Failed to init IPP FFT op." ) );
//    }

//    // Allocate reusable resource for ipp. This speeds up IPP by avoiding
//    // an internal reallocation at each step.
//    int fftWorkBufSize = 0;
//    ippsFFTGetBufSize_R_16s( fftData.fftSpec, &fftWorkBufSize );

//    fftData.fftWorkBuf = fftWorkBufSize > 0 ? new char[ fftWorkBufSize ] : NULL;

//    if( fftData.fftWorkBuf == NULL )
//    {
//        emit sendError( tr( "Failed to allocate FFT Work buffer" ) );
//    }
}

/*
 * transformData
 *
 * FFT the input laser signal (frequency to spatial domain) and apply
 * 10log10 scaling.
 */
bool SoftwareDSP::transformData_sw( unsigned short *data )
{
//    TIME_THIS_SCOPE( dsp_transformData );

//    IppStatus status = ippStsNoErr;

//    // copy the rescaled data into the working fft buffer
//    ippsCopy_16s( (Ipp16s *)data, (Ipp16s *)fftData.fftChABuf, fftData.inputSize );

//    // Window in place (possible optimization: may be faster to pre-compute window and multiply)
//    ippsWinHann_16s_I( (Ipp16s *)fftData.fftChABuf, fftData.inputSize );

//    // Do an in-place, real only, 16-bit fft
//    status = ippsFFTFwd_RToCCS_16s_ISfs( (Ipp16s *)fftData.fftChABuf,
//                                          fftData.fftSpec,
//                                          fftData.fftScaleFactor,
//                                          (Ipp8u *)fftData.fftWorkBuf );

//    if( status != ippStsNoErr )
//    {
//        LOG( FATAL, "Channel A fft operation failed." );
//        return false;
//    }

//    // Compute the magnitude of the now complex FFT data.
//    status = ippsMagnitude_16sc_Sfs( (Ipp16sc *)fftData.fftChABuf,
//                                     (Ipp16s *)fftData.fftChABuf,
//                                     fftData.fftPoints / 2,
//                                     fftData.magScaleFactor );

//    if( status != ippStsNoErr )
//    {
//        LOG( FATAL, "Channel A magnitude operation failed." );
//        return false;
//    }

//    /*
//     * Apply log scaling to the FFT Magnitude. Magnitude is calculated on the first 1024 points.
//     * Only need to calculate the log on the first 1024 points.
//     */
//    for( unsigned int i = 0; i < ( fftData.fftPoints / 2 ); i++ )
//    {
//        // Account for scale factor used in above transforms and apply log
//        data[ i ] = logTable[ fftData.fftChABuf[ i ] ];
//    }

//    // zero-out the DC component
//    data[ 0 ] = 0;

//    // Update frame-specific information
//    timeStamp    = QDateTime::currentDateTime().toUTC().toTime_t();
//    milliseconds = QTime::currentTime().msec();

    return true;
}


/*
 * rescale
 *
 * rescale() operates on input data to produce data which is *as if* the data
 * had been sampled linearly in frequency. Given a set of ideally linear in frequency
 * sample numbers (which are actually floating point, so non-existant), the function
 * interpolates between existing data to produce approximate samples.
 *
 * The input data is assumed to be zeroed before this function is called.
 * offset is subtracted from the final value (if provided) to correct for any
 * DAQ ADC count offset. It may be removed if not deemed necessary.
 *
 * Consider using IPP for speedup if performance seems to be an issue.
 */
unsigned int SoftwareDSP::rescale( const unsigned short *inputData )
{
    TIME_THIS_SCOPE( dsp_rescale );

    unsigned int outIndex;
    unsigned int sampleIndex;
    double       interpSample;

    // baby steps
    unsigned int    inputLength   = recordLength;
    unsigned int    rescaleLength = DSP::RescalingDataLength;
    unsigned short *outputData    = resampledData;
    unsigned int    offset        = 0 ;

    // Sanity check
    if( !inputData || !wholeSamples || !fractionalSamples || !outputData )
    {
        return 0;
    }

    // clear out the buffer memory before filling it. This prevents
    // configuration where the rescaling values are longer than the
    // input data length.
    memset( outputData, 0, rescaleLength * sizeof( unsigned short ) );

    for( outIndex = 0; outIndex < rescaleLength; outIndex++ )
    {
        // Extract the index of the sample we are using as the first data point
        sampleIndex = U32(wholeSamples[ outIndex ]);

        // Does the rescaling data point beyond what we've captured? If so, bail.
        if( ( sampleIndex + 1 ) > inputLength )
        {
            break;
        }

        // Linearly interpolate between that sample and the next one, according to
        // where whole + fractional is located.
        interpSample = ( ( double(inputData[ sampleIndex + 1 ]) - double(inputData[ sampleIndex ] )) * double(fractionalSamples[ outIndex ]) );
        interpSample = interpSample + double(inputData[ sampleIndex ]) - double(offset);
        outputData[ outIndex ] = U16(floor_int( float(interpSample) ));
    }

    return outIndex;
}
/*
 * computeLogTable
 *
 * Description: The logarithm is one of the most expensive computational
 *    operations we have to perform. IPP does not provide a general
 *    logarithm function that applies to our situation. Luckily, we are
 *    dealing with 16-bit integers, so we can use a lookup table approach
 *    to significantly speed this operation up. For one channel the total
 *    storage is  2 * 65535 = 128k, which is quite reasonable.
 *
 *    This function computes the log lookup table, based on the configured
 *    scaling factors.
 */
void SoftwareDSP::computeLogTable( void )
{
    // Reallocate if already done before
    if( logTable )
    {
        delete [] logTable;
    }
    logTable = new unsigned short[ LOG_TABLE_SIZE ];

    if( logTable )
    {
        // Fatal error
        emit sendError( tr( "Could not allocate memory for the DSP log table" ) );
    }

    // 0 input clamped to 0dB
    logTable[ 0 ] = 0;

    // Compute (10 * log10(i))*scale of every possible input
    for( int i = 1; i < LOG_TABLE_SIZE; i++ )
    {
        logTable[ i ] = U16( 10.0 * log10( double( i ) * LOG_SCALE_FACTOR ));
    }
}


/*
 * adjustForDisplay
 *
 * Description: Given the FFT'd signal, scale and contrast stretch for
 *              greyscale (8-bit) display.
 *
 * Return value: true on success, false otherwise.
 */
bool SoftwareDSP::adjustForDisplay( const unsigned short * const pFftData,
                                    unsigned char * pDisplayData,
                                    const int BufferSize )
{
//    TIME_THIS_SCOPE( dsp_adjustForDisplay );

//    // ipp doesn't respect the const pointer and const data input; copy
//    // the working data into a temporary buffer
//    ippsCopy_16s( (Ipp16s *)pFftData, (Ipp16s *)adjustBuffer, BufferSize );

//    // Scale based on black and white levels. Subtract off the black level.
//    IppStatus retval = ippsSubC_16u_ISfs( blackLevel, (Ipp16u *)adjustBuffer, BufferSize, 0 );
//    if( retval != ippStsNoErr )
//    {
//        LOG( FATAL, "Failed to offset FFT output" );
//        return false;
//    }

//    // This scale factor, when divided into the data, will now
//    // fit the 16-bit data between the above limits into an 8-bit
//    // representation.
//    unsigned short scaleFactorA = (unsigned short)ceil( (double)( whiteLevel - blackLevel ) /
//                                                        (double)FullScaleIntensityValue );

//    // No infinities
//    if( abs( scaleFactorA ) < 1 )
//    {
//        scaleFactorA = 1;
//    }

//    // Divide in scale factor
//    retval = ippsDivC_16u_ISfs( scaleFactorA, (Ipp16u *)adjustBuffer, BufferSize, 0 );

//    if( retval != ippStsNoErr )
//    {
//        LOG( FATAL, QString( "Failed to scale FFT output: %1" ).arg( retval ) );
//        return false;
//    }

//    // Convert to 8-bit.
//    //    -- no ippsConvert_* available for unsigned short to unsigned char
//    for( int i = 0; i < BufferSize; i++ )
//    {
//        // make sure values over the maximum are set to full-scale (white); just
//        // masking the lower bits does not give full-scale values for
//        // data > FullScaleIntensityValue
//        if( adjustBuffer[ i ] > FullScaleIntensityValue )
//        {
//            adjustBuffer[ i ] = FullScaleIntensityValue;
//        }
//        if( doInvertColors )
//        {
//            pDisplayData[ i ] = FullScaleIntensityValue - (unsigned char)( adjustBuffer[ i ] & 0xFF );
//        }
//        else
//        {
//            pDisplayData[ i ] = (unsigned char)( adjustBuffer[ i ] & 0xFF );
//        }

//    }

    return true;
}


/*
 * computeEncoderAngle
 *
 * Angle returned is in encoder counts (0 to Lines Per Revolution).
 *
 * This method handles encoders with integer gearing rations between the catheter
 * and the encoder.  The current handle design uses a 1:1 ratio so this is
 * a little overkill but computationally it is insignificant compared to other
 * processes that are going on.
 *
 */
int SoftwareDSP::computeEncoderAngle( unsigned short *data, int length )
{
//    TIME_THIS_SCOPE( dsp_computeEncoderAngle );

//    // Max difference allowed between measurements at the start and finish
//    // of the data record
//    const int MaxSlewDiff_cnt = 300;

//    // The BaseOffset is for geared encoders only. This is
//    const unsigned short BaseOffset = encoderSettings.linesPerRev / encoderSettings.gearingRatio;

//    // Used to determine when the encoder has crossed the 0/360 transition
//    const int WrapAngle_cnt = 0.75 * encoderSettings.linesPerRev;

//    // used to determine encoder min and max values for this session
//    static unsigned short prevMax = encoderSettings.calMaxVal;
//    static unsigned short prevMin = encoderSettings.calMinVal;

//    unsigned short rawAverage = 0;
//#if DEBUG_ENCODER_CROSSING
//    static unsigned short prevRawAverage = 0; // DWJ debugging encoder streaking around 0
//#endif

//    // The slew rate of the encoder signal is slower than the sampling rate. If
//    // the difference is large, use the previous angle
//    if( abs( (int)data[ 0 ] - (int)data[ length - 1 ] ) > MaxSlewDiff_cnt )
//    {
//        currAngle.angle_cnt = prevAngle.angle_cnt;
//        qDebug() << "Encoder: value rejected for slew rate";
//    }
//    else
//    {
//        // Compute the average of the samples
//        ippsMean_16s_Sfs( (Ipp16s *)data, length, (Ipp16s *)&rawAverage, 0);

//#if USE_SW_ENCODER
//        // "Rotate" the encoder
//        static int i = encoderSettings.calMinVal;
//        rawAverage = ( i += 5 );
//        if( i > ( encoderSettings.calMaxVal ) ) i = encoderSettings.calMinVal;
//        if( i < ( encoderSettings.calMinVal ) ) i = encoderSettings.calMaxVal;
//#endif

//        // if updates are allowed and either value has changed, update the previous states
//        if( numTimesWrapped < MaxEncoderChecks )
//        {
//            // determine the min/max for the encoder
//            encoderSettings.calMaxVal = max( encoderSettings.calMaxVal, rawAverage );
//            encoderSettings.calMinVal = min( encoderSettings.calMinVal, rawAverage );

//            if( ( encoderSettings.calMaxVal != prevMax ) || ( encoderSettings.calMinVal != prevMin ) )
//            {
//                prevMax = encoderSettings.calMaxVal;
//                prevMin = encoderSettings.calMinVal;
//            }
//        }
//        else
//        {
//            // pull values in range after min/max have been set
//            if( rawAverage > encoderSettings.calMaxVal )
//            {
//#if DEBUG_ENCODER_CROSSING
//                LOG( DEBUG, QString( "Max value trapped. Was %1, set to %2," ).arg( rawAverage ).arg( encoderSettings.calMaxVal ) );
//#endif
//                rawAverage = encoderSettings.calMaxVal;
//            }
//            if( rawAverage < encoderSettings.calMinVal )
//            {
//#if DEBUG_ENCODER_CROSSING
//                LOG( DEBUG, QString( "Min value trapped. Was %1, set to %2," ).arg( rawAverage ).arg( encoderSettings.calMinVal ) );
//#endif
//                rawAverage = encoderSettings.calMinVal;
//            }
//        }

//        // Log the values that will be used for this device for the rest of the session
//        if( logEncoderValues && ( numTimesWrapped == MaxEncoderChecks ) )
//        {
//            LOG( INFO, QString( "Device Encoder Settings (counts): Min: %1  Max: %2" ).arg( encoderSettings.calMinVal ).arg( encoderSettings.calMaxVal ) );
//            logEncoderValues = false;
//        }

//        // convert the A/D value to an encoder line value (full range is 0 to (linesPerRev - 1))
//        // This formula is odd looking but it works very well - the full A/D range is evenly distributed
//        // and wholly contained in the encoder range. Other mappings that seem more intuitively correct
//        // suffer from uneven distributions into the edge encoder values or out of range values.
//        // This formula handles the edge conditions nicely.
//        currAngle.rawAngle_cnt = ( encoderSettings.linesPerRev * ( rawAverage - encoderSettings.calMinVal + 1 ) ) /
//                                 ( encoderSettings.calMaxVal - encoderSettings.calMinVal + 2 );

//        int dRawAngle_cnt = prevAngle.rawAngle_cnt - currAngle.rawAngle_cnt;

//        // Check the change in raw angle from the previous raw angle. If it
//        // is greater than the WrapAngle we conclude that we crossed the
//        // 0/360 transition.  Compute the offset to add to the computed
//        // angle. If the encoder is geared to the catheter, the encoder will
//        // wrap more than once per 360 deg rotation of the catheter; the base
//        // offset (one encoder revolution for this gearing ratio) is added or
//        // subtracted from the tracked offset to add to the computed angle.
//        if( abs( dRawAngle_cnt ) > WrapAngle_cnt )
//        {
//            currAngle.offset_cnt = prevAngle.offset_cnt + sign( dRawAngle_cnt ) * BaseOffset;
//            numTimesWrapped++;

//            /*
//             * Compute the difference from this wrap event to the previous one and compute
//             * the rotational velocity.
//             */
//            currTime = encoderTimer.elapsed();

//            // save state
//            prevTime = currTime;

//#if DEBUG_ENCODER_CROSSING
//            int range = abs( (int)rawAverage - (int)prevRawAverage );
//            LOG( DEBUG, QString( "Transition: prevRawAverage: %1,  rawAverage: %2, range: %3," ).arg( prevRawAverage ).arg( rawAverage ). arg( range ) );
//#endif
//        }
//        else
//        {
//            currAngle.offset_cnt = prevAngle.offset_cnt;
//        }

//#if DEBUG_ENCODER_CROSSING
//        prevRawAverage = rawAverage;
//#endif

//        // Compute the actual angle: the angle within the gearing plus the
//        // gearing offset; clamp it to +/- 360 deg
//        currAngle.angle_cnt = ( ( currAngle.rawAngle_cnt / encoderSettings.gearingRatio )
//                                + currAngle.offset_cnt ) % encoderSettings.linesPerRev;

//        // Make sure the angle is positive
//        if( currAngle.angle_cnt < 0 )
//        {
//            currAngle.angle_cnt += encoderSettings.linesPerRev;
//        }

//        // Save states for the next run
//        prevAngle = currAngle;
//    }

//    return currAngle.angle_cnt;
    return 0;//lcv
}


/*
 * setEncoderConfigurationToDefault
 *
 * Set the encoder settings to default values.  This is called at start-up
 * or any time the device is changed.
 */
void SoftwareDSP::setEncoderConfigurationToDefault( void )
{
    encoderSettings  = defaultEncoderSettings;
    numTimesWrapped  = 0;
    logEncoderValues = true;
}

#if ENABLE_IPP_FFT_TUNING
void SoftwareDSP::setMagScaleFactor( int val )
{
    fftData.magScaleFactor = val;
}

void SoftwareDSP::setFftScaleFactor( int val )
{
    fftData.fftScaleFactor = val;
}
#endif


#if ENABLE_LOW_SPEED_DATA_SNAPSHOT
#include <QFile>
#include "Utility/userSettings.h"

/*
 * saveSignals
 *
 * Save the resampled raw and the post-FFT signals to CSV for R&D use.
 */
void SoftwareDSP::saveSignals()
{
    static int fileIteration = 0;
    QString strIteration = QString( "%1" ).arg( fileIteration, 3, 10, QLatin1Char( '0' ) );

    caseInfo &info = caseInfo::Instance();
    const QString StorageDir = info.getStorageDir();

    QString rescaledDataFilename = QString( StorageDir + "/lowSpeedData-%1-resampled.csv" ).arg( strIteration );

    errorHandler & err = errorHandler::Instance();
    QFile *hRescaledDataFile = new QFile( rescaledDataFilename );
    if( hRescaledDataFile == NULL )
    {
        // fatal error
        err.fail( tr( "Could not create a new file handle for saving OCT data." ), true );
    }
    if( !hRescaledDataFile->open( QIODevice::WriteOnly ) )
    {
        err.fail( tr( "Could not open OCT file for writing." ), true );
    }

    QTextStream outRescaledDataStream;
    outRescaledDataStream.setDevice( hRescaledDataFile );

    /*
     * Save the resampled raw data
     */
    for( int i = 0; i < DSP::RescalingDataLength; i++ )
    {
        outRescaledDataStream << pData->rawData[ i ] << "\r\n";
    }

    if( hRescaledDataFile )
    {
        hRescaledDataFile->flush();
        hRescaledDataFile->close();
        delete hRescaledDataFile;
        hRescaledDataFile = NULL;
    }

    QString postFFTFilename = QString( StorageDir + "/lowSpeedData-%1-postFFT.csv" ).arg( strIteration );
    QFile *hPostFFTFile = new QFile( postFFTFilename );
    if( hPostFFTFile == NULL )
    {
        // fatal error
        err.fail( tr( "Could not create a new file handle for saving OCT data." ), true );
    }
    if( !hPostFFTFile->open( QIODevice::WriteOnly ) )
    {
        err.fail( tr( "Could not open the text file for writing." ), true );
    }

    QTextStream outPostFFTDataStream;
    outPostFFTDataStream.setDevice( hPostFFTFile );

    /*
     * Save the post-FFT data
     */
    for( int i = 0; i < FFTDataSize; i++ )
    {
        outPostFFTDataStream << pData->fftData[ i ] << "\r\n";
    }

    if( hPostFFTFile != NULL )
    {
        hPostFFTFile->flush();
        hPostFFTFile->close();
        delete hPostFFTFile;
        hPostFFTFile = NULL;
    }

    fileIteration++;
}
#endif
