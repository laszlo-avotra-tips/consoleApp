/*
 * dspgpu.cpp
 *
 * Handle raw signal data from the DAQ
 *    - Resample raw laser data
 *    - Process A-lines
 *    - Encoder angle
 *    - Scale output for display
 *
 * Implements a Santec-compatible frequency rescaling algorithm (rescale()).
 * This function takes data which has been sampled at regular time intervals
 * (via a DAQ) and interpolates it to produce data that appears sampled at
 * regular frequency intervals, according to a frequency/time interval function.
 *
 * The input data is expected to be in two arrays of double precision, the first
 * are the whole parts of the sample numbers appearing at linear frequency intervals.
 * The second contains the fractional parts. The code then (linearly) interpolates the data
 * values at the whole + fractional sample number based on the preceding and following
 * whole samples.
 *
 * Author(s): Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include "clAmdFft.h"
#include "defaults.h"
#include "dspgpu.h"
#include "logger.h"
#include "profiler.h"
#include "util.h"
#include "buildflags.h"
#include "deviceSettings.h"
#include "Backend/depthsetting.h"
#include "ipp.h"
#include "theglobals.h"


#if _DEBUG
#   define RESCALE_CL  "./backend/opencl/rescale.cl"
#   define POSTPROC_CL "./backend/opencl/postproc.cl"
#   define BANDC_CL    "./backend/opencl/bandc.cl"
#   define WARP_CL     "./backend/opencl/warp.cl"
#else
#   define RESCALE_CL  "./rescale.cl"
#   define POSTPROC_CL "./postproc.cl"
#   define BANDC_CL    "./bandc.cl"
#   define WARP_CL     "./warp.cl"
#endif

/*
 * OpenCL core organization parameters
 */
#define DEFAULT_GLOBAL_UNITS ( 2048 )
#define DEFAULT_LOCAL_UNITS  ( 16 )

size_t global_unit_dim[] = { DEFAULT_GLOBAL_UNITS, DEFAULT_GLOBAL_UNITS };
size_t local_unit_dim[]  = { DEFAULT_LOCAL_UNITS,  DEFAULT_LOCAL_UNITS  };

// Normalize distances in pixels to (1/2) SectorWidth, makes distances in the range of 0.0->1.0
const float NormalizeScalingFactor = (float)( (float)SectorWidth_px / (float)2 );

/*
 * destructor
 *
 * The DSP base constructor is called implicitly
 */
DSPGPU::~DSPGPU()
{
    LOG( INFO, "DSP GPU shutdown" );
    qDebug() << "DSPGPU::~DSPGPU()";

    // Release working memory
    for( int i = 0; i < channelCount; i++ )
    {
        if( workingBuffer[ i ] != NULL )
        {
            free( workingBuffer[ i ] );
            workingBuffer[ i ] = NULL;
        }
    }

    if( imData )
    {
        delete [] imData;
    }

    if( reData )
    {
        delete [] reData;
    }

    if( windowBuffer )
    {
        free( windowBuffer );
    }

    if( fftImaginaryBuffer )
    {
        free( fftImaginaryBuffer );
    }

    if( pPostProcOutputFrame )
    {
        delete [] pPostProcOutputFrame;
    }

    /*
     * Clean up openCL objects
     */
    clAmdFftTeardown();
    clReleaseKernel( cl_RescaleKernel );
    clReleaseKernel( cl_PostProcKernel );
    clReleaseKernel( cl_BandCKernel );
    clReleaseKernel( cl_WarpKernel );

    clReleaseProgram( cl_RescaleProgram );
    clReleaseProgram( cl_PostProcProgram );
    clReleaseProgram( cl_BandCProgram );
    clReleaseProgram( cl_WarpProgram );

    clReleaseMemObject( rescaleInputMemObj );
    clReleaseMemObject( rescaleOutputMemObj );
    clReleaseMemObject( rescaleFracSamplesMemObj );
    clReleaseMemObject( rescaleWholeSamplesMemObj );

    clReleaseMemObject( fftImaginaryInputMemObj );
    clReleaseMemObject( postProcOutputMemObj );

    clReleaseMemObject( windowMemObj );
    clReleaseMemObject( fftRealOutputMemObj );
    clReleaseMemObject( fftImaginaryOutputMemObj );

    clReleaseMemObject( lastFramePreScalingMemObj );

    clReleaseMemObject( inputImageMemObj );
    clReleaseMemObject( warpInputImageMemObj );
    clReleaseMemObject( outputImageMemObj );
    clReleaseMemObject( outputVideoImageMemObj );

    clReleaseCommandQueue( cl_Commands );
    clReleaseContext( cl_Context );
}

/*
 * init
 *
 * Initialize memory and the GPU hardware
 */
void DSPGPU::init( unsigned int inputLength,
                   unsigned int frameLines,
                   int inBytesPerRecord,
                   int inBytesPerBuffer,
                   int inChannelCount )
{
    qDebug() << "DSPGPU::init";

    // call the common initilization steps
    DSP::init( inputLength, frameLines, inBytesPerRecord, inBytesPerBuffer, inChannelCount );

    qDebug() << "DSP: Allocating space for workingBuffer:" << bytesPerBuffer << "B";

    LOG1(bytesPerBuffer);

    for( int i = 0; i < inChannelCount; i++ )
    {
        // XXX Need to pass these into the DSP so we dont copy buffers there
        // for opencl
        workingBuffer[ i ] = (U16 *)malloc( bytesPerBuffer );
    }

    if( ( workingBuffer[ 0 ] == NULL ) ||
        ( ( inChannelCount == 2 ) && ( workingBuffer[ 1 ] == NULL ) ) )
    {
        emit sendError( tr( "Error allocating space in DSP::init" ) );
    }

    qDebug() << "DSPGPU: Allocating space for PostProcOutputBuffer";
    pPostProcOutputFrame = new unsigned char[ linesPerFrame * MaxALineLength ]; // take full buffer 1024 * linesPerFrame

    // Configure the DSP
    computeFFTWindow();

    if( !initOpenCL() )
    {
        displayFailureMessage( tr( "Could not initialize OpenCL." ), true );
    }

    doAveraging      = false;
    displayAngle_deg = 0.0;

    currFrameWeight_percent = DefaultCurrFrameWeight_Percent / 100.0;
    prevFrameWeight_percent = 1 - currFrameWeight_percent;

    imData = new float[ 1024 ];
    reData = new float[ 1024 ];
}

/*
 * processData
 *
 * Do all the work we need to do to a frame of A-line data before handing it off to be displayed
 *    - Rescale the data according to the laser characterization data for fine-tuning the waveform
 *    - Transform the data (FFT, magnitude, log)
 *    - Bundle all of the data up for displaying and storing
 *
 * Data is shared across threads via a global data buffer.  A single counter indexes
 * this buffer.  The DAQ writes to a slot in the buffer while the consumer alwayes
 * reads one slot behing the current index.  This buffer is lossy on purpose; the
 * laser produces more lines than we can transfer and display; there is no loss of
 * clinically relevant data since many of the lines are of the same view.
 */
void DSPGPU::processData( void )
{
    // initialize prevIndex to the same value that index will get so no
    // work is done until the DAQ starts up
     static int prevIndex = TheGlobals::instance()->getPrevGDaqRawData_idx();

    // which index to point into for the raw data
     int index = TheGlobals::instance()->getPrevGDaqRawData_idx();

    // Only process data if it has been updated
//#if QT_NO_DEBUG
    if( index != prevIndex )
//#endif
    {
        TIME_THIS_SCOPE( DSPGPU_processData );

        // determine where to store this frame of data for the Frontend
        pData = TheGlobals::instance()->getFrameDataPointer();

        // use local pointer into data for easier access
        const U16 *pA = NULL;

        if( channelCount == 1 )
        {
            pA = TheGlobals::instance()->getDaqRawDataBuffer(size_t(index));
        }
        else
        {
            // ATS DAQ interleaves Channels A & B.
            const U16 *pBothChannels = TheGlobals::instance()->getDaqRawDataBuffer(size_t(index));

            // point to the start of each working buffer
            auto wb0 = workingBuffer[ 0 ];
            auto wb1 = workingBuffer[ 1 ];

            // De-interleave the data. IPP has a deinterleave function but my first
            // pass at it failed. Might be faster; doesn't appear to matter.
            for( unsigned int i = 0, j = 0; i < ( bytesPerBuffer / 2 ); i += 2, j++ )
            {
                wb0[ j ] = pBothChannels[ i ];
                wb1[ j ] = pBothChannels[ i + 1 ];
            }
        }

#if !ENABLE_DEMO_MODE
        // walk through the bulk data handling
        if( rescale( pA ) != 0 )  //Success if return 0
        {
            LOG( WARNING, "Failed to rescale data on GPU." );
        }
        if( !transformData( pData->dispData, pData->videoData ) )   //Success if return true
        {
            LOG( WARNING, "Failed to transform data on GPU." );
        }
#else
        // zero out the image data, no processing for demo mode
        memset( pData->dispData, 0, SectorHeight_px * SectorWidth_px * sizeof( unsigned char ) );
#endif

        // Copy the resampled raw data into the shared memory buffer. The buffer
        // resampledData is used for in-place transforming of the data so it must be
        // copied before any other processing takes place.
        ippsCopy_16s( (Ipp16s *)pA, (Ipp16s *)pData->rawData, bytesPerRecord / 2 );  // used for Advanced View display
//lcv        ippsCopy_8u( reinterpret_cast<const U8*>(pA), reinterpret_cast<U8*>(pData->rawData), bytesPerRecord );

#if ENABLE_RAW_DATA_SNAPSHOT
        if( isRecordRaw && ( rawCount < snapshotLength ) )
        {
            rawOutputStream.writeRawData( (const char *)pA, bytesPerBuffer );
            rawCount++;
            
            qDebug() << "rawcount"<< rawCount;
            if( rawCount >= snapshotLength )
            {
                qDebug() << "closing raw data";
                closeRawDataFiles();
            }
        }
#endif

        /*
         * Frame counts are filled in by the Data Consumer;
         * this keeps the frame counts continuous.
         */
        pData->frameCount      = 0;
        pData->encoderPosition = 0; // NOT USED for fast-OCT
        pData->timeStamp       = getTimeStamp();
        pData->milliseconds    = getMilliseconds();

        // Update the global index into the shared buffer. This must be the last thing in the thread
        // to make sure that the consumer thread only sees completely filled data structures.
        TheGlobals::instance()->inrementGFrameCounter();

//#ifdef QT_DEBUG
//        TheGlobals::instance()->incrementGDaqRawDataCompleted();
//        TheGlobals::instance()->updateGDaqRawData_idx();
//#endif
        prevIndex = index;
    }

    yieldCurrentThread();
}


/*
 * ComputeFFTWindow
 *
 * Precompute the window function to be applied to the pre-FFT signal.
 */
void DSPGPU::computeFFTWindow( void )
{
    windowBuffer = (float *)malloc( RescalingDataLength * sizeof(float) );
    ippsSet_32f( 1, windowBuffer, RescalingDataLength );
    ippsWinHann_32f_I( windowBuffer, RescalingDataLength );
}

/*
 * initOpenCLFFT
 *
 * Set up the FFT plan for using clAmdFft.
 *
 */
bool DSPGPU::initOpenCLFFT( void )
{
    clAmdFftStatus status;
    clAmdFftSetupData setupData;

    status = clAmdFftInitSetupData( &setupData );
    if( status != CLFFT_SUCCESS )
    {
        qDebug() << "clAmdFFtInitSetupData failed: " << status;
        return false;
    }

    status = clAmdFftSetup( &setupData );
    if( status != CLFFT_SUCCESS )
    {
        qDebug() << "clAmdFFtSetup failed: " << status;
        return false;
    }

    size_t fftLen = DSP::RescalingDataLength;
    status = clAmdFftCreateDefaultPlan( &hCl_fft_plan, cl_Context,
                                        CLFFT_1D, &fftLen );
    if( status != CLFFT_SUCCESS )
    {
        qDebug() << "clAmdFFtCreateDefaultPlan failed: " << status;
        return false;
    }

    status = clAmdFftSetLayout( hCl_fft_plan, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR );
    if( status != CLFFT_SUCCESS)
    {
        qDebug() << "clAmdFftSetLayout failed: " << status;
        return false;
    }

    status  =  clAmdFftSetPlanBatchSize( hCl_fft_plan, linesPerFrame );
    if( status != CLFFT_SUCCESS)
    {
        qDebug() << "clAmdFftSetPlanBatchSize failed: " << status;
        return false;
    }

    status = clAmdFftSetPlanDistance( hCl_fft_plan, RescalingDataLength, RescalingDataLength );
    if( status != CLFFT_SUCCESS)
    {
        qDebug() << "clAmdFftSetPlanDistance failed: " << status;
        return false;

    }

    status = clAmdFftSetResultLocation( hCl_fft_plan, CLFFT_OUTOFPLACE );
    if( status != CLFFT_SUCCESS )
    {
        qDebug() << "clAmdFftSetResultLocation failed: " << status;
        return false;
    }

    status = clAmdFftBakePlan( hCl_fft_plan, 1, &cl_Commands, NULL, NULL );
    if( status != CLFFT_SUCCESS)
    {
        qDebug() << "clAmdFftBakePlan failed: " << status;
        return false;
    }

    fftImaginaryBuffer = (float *)malloc( sizeof(float) * RescalingDataLength * linesPerFrame );
    if( !fftImaginaryBuffer )
    {
        qDebug() << "malloc() fftImaginaryBuffer failed.";
        return false;
    }
    memset( fftImaginaryBuffer, 0, sizeof(float) * RescalingDataLength * linesPerFrame );
    return true;
}

/*
 * buildOpenCLKernel
 *
 * Load, compile and build the program for the given OpenCL kernel.
 */
bool DSPGPU::buildOpenCLKernel( QString clSourceFile, const char *kernelName, cl_program *program, cl_kernel *kernel )
{
    qDebug() << "DSPGPU::buildOpenCLKernel:" << clSourceFile;
    QTime buildTimer;
    buildTimer.start();

    int err;

    /*
     * Load, compile, link the source
     */
    LOG2(clSourceFile,kernelName);
    char *sourceBuf = loadCLProgramSourceFromFile( clSourceFile ); // XXX: We should switch to pre-compiled binary. See #1057
    if( !sourceBuf )
    {
        displayFailureMessage( tr( "Failed to load program source file %1 (%2)" ).arg( clSourceFile ).arg( QDir::currentPath() ), true );
        return false;
    }

    /*
     * Create the compute program(s) from the source buffer
     */
    *program = clCreateProgramWithSource( cl_Context, 1, (const char **) &sourceBuf, NULL, &err );
    if( !*program || ( err != CL_SUCCESS ) )
    {
        qDebug() << "DSP: OpenCL could not create program from source: " << err;
        displayFailureMessage( tr( "Could not build OpenCL kernel from source, reason %1" ).arg( err ), true );
        return false;
    }
    free( sourceBuf );

    err = clBuildProgram( *program, 0, NULL, NULL, NULL, NULL );
    if( err != CL_SUCCESS )
    {
        size_t length;
        const int BuildLogLength = 2048;
        char *build_log = (char *)malloc( BuildLogLength );

        qDebug() << "DSP: OpenCL build failed: " << err;
        clGetProgramBuildInfo( *program, cl_ComputeDeviceId, CL_PROGRAM_BUILD_LOG, BuildLogLength, build_log, &length );
        qDebug() << "openCl Build log:" << build_log;

        displayFailureMessage( tr( "Could not build program, reason %1" ).arg( err ), true );
        free( build_log );
        return false;
    }

    *kernel = clCreateKernel( *program, kernelName, &err );

    if( err != CL_SUCCESS )
    {
        qDebug() << "DSP: OpenCL could not create compute kernel: " << err;
        displayFailureMessage( tr( "Could not create compute kernel, reason %1" ).arg( err ), true );
        return false;
    }
    LOG1( buildTimer.elapsed());
    qDebug() << "Build time:" << buildTimer.elapsed() << "ms";
    return true;
}

/*
 * initOpenCL
 *
 * Enumerate and initialize OpenCL devices, programs and kernels. This
 * function is very long and only gets worse with more kernels. Think
 * about how to break it up. Maybe this function is table driven and
 * loads all kernels from a structure?
 */
bool DSPGPU::initOpenCL()
{
    qDebug() << "initOpenCL start";

#if _DEBUG
    QString path = QCoreApplication::applicationDirPath();
#else
    QString path = QCoreApplication::applicationDirPath();
#endif

    cl_platform_id platformId;
    cl_uint        numPlatforms = 0;

    int err = clGetPlatformIDs( 0, NULL, &numPlatforms );
    qDebug() << "numPlatforms =" << numPlatforms;

    if( numPlatforms == 0 )
    {
        // fatal error
        displayFailureMessage( tr( "Could not find openCL platform, reason: %1" ).arg( err ), true );
        return false;
    }

    // Found openCL-capable platforms
    cl_platform_id* platformIds = ( cl_platform_id* )malloc( sizeof( cl_platform_id ) * numPlatforms );
    err = clGetPlatformIDs( numPlatforms, platformIds, NULL );

    int deviceIndex = -1;

    const int DefaultStringSize = 128;
    char vendor[ DefaultStringSize ];
    char name[ DefaultStringSize ];
    char version[ DefaultStringSize ];


    for ( cl_int i = 0; i < numPlatforms; i++ )
    {
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VENDOR,  DefaultStringSize, vendor,  NULL );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_NAME,    DefaultStringSize, name,    NULL );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VERSION, DefaultStringSize, version, NULL );

        if ( err != CL_SUCCESS )
        {
            displayFailureMessage( tr( "Could not enumerate OpenCL platform IDs, reason: %1" ).arg( err ), true );
            return false;
        }
        qDebug() << "Platform (" << i << ") Vendor:" << vendor << " Name:" << name << " Version:" << version;

        if ( QString( vendor ) == "Advanced Micro Devices, Inc." &&
             QString( name )   == "AMD Accelerated Parallel Processing" )
        {
            deviceIndex = i;
        }

#ifdef QT_DEBUG
        if ( QString( vendor ) == "NVIDIA Corporation" && (deviceIndex == -1))
        {
            deviceIndex = i;
        }
#endif
        LOG2(i,deviceIndex);
        if(i == deviceIndex){
            LOG3(vendor,name,version);
        }
    }

    // Fatal error if no AMD-compatible platforms are found
//#if QT_NO_DEBUG //lcv
    if ( deviceIndex < 0 )
    {
        displayFailureMessage( tr( "Could not find AMD platform" ), true );
        return false;
    }
//#endif
    platformId = platformIds[ deviceIndex ];

    // release the memory.  Error paths do not free the memory since they will shut down the program
    free( platformIds );

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Could not enumerate OpenCL platform IDs, reason: %1" ).arg( err ), true );
        return false;
    }

    // Verify the GPU is present
    err = clGetDeviceIDs( platformId, CL_DEVICE_TYPE_GPU, 1, &cl_ComputeDeviceId, NULL );

    // If not, fall back to the CPU. Display a warning if this occurs on the release hardware
    if( err == CL_DEVICE_NOT_FOUND )
    {
        // fall back to CPU when debugging if GPU not present
#if !_DEBUG
#if !ENABLE_SQUISH
        displayWarningMessage( tr( "GPU not present.  Falling back to the CPU." ) );
#endif
        qDebug() << "GPU not present.  Falling back to the CPU.";
#endif
        err = clGetDeviceIDs( platformId, CL_DEVICE_TYPE_CPU, 1, &cl_ComputeDeviceId, NULL );
    }

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Could not get OpenCL device IDs, reason: %1" ).arg( err ), true );
        return false;
    }

    {
//        cl_int clGetDeviceInfo (cl_device_id device,
//         cl_device_info param_name,
//         size_t param_value_size,
//         void *param_value,
//         size_t *param_value_size_ret)
    }

    size_t returned_size( 0 );
    cl_uint maxComputeUnits;
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, &returned_size );
    LOG3(err, maxComputeUnits,returned_size);

    cl_uint maxWorkItemDimentions;
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxWorkItemDimentions), &maxWorkItemDimentions, &returned_size );
    LOG3(err, maxWorkItemDimentions,returned_size);

    size_t maxWorkItemSizes[3];
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, &returned_size );
    LOG4(err, maxWorkItemSizes[0],maxWorkItemSizes[1],maxWorkItemSizes[2]);

    err = clGetDeviceInfo( cl_ComputeDeviceId,
                           CL_DEVICE_MAX_WORK_GROUP_SIZE,
                           sizeof( cl_max_workgroup_size ),
                           &cl_max_workgroup_size,
                           &returned_size );
    LOG3(err, cl_max_workgroup_size, returned_size);

    size_t maxWorkGroupSize;
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, &returned_size );
    LOG3(err, maxWorkGroupSize,returned_size);

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Could not enumerate OpenCL device IDs, reason: %1" ).arg( err ), true );
        return false;
    }

    cl_char vendor_name[ 1024 ] = { 0 };
    cl_char device_name[ 1024 ] = { 0 };
    err  = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_VENDOR, sizeof( vendor_name ), vendor_name, &returned_size);
    err |= clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_NAME, sizeof( device_name ), device_name, &returned_size);

    LOG2( reinterpret_cast<char*>(vendor_name), reinterpret_cast<char*>(device_name));

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Could not get OpenCL device info, reason: %1").arg( err ), true );
        return false;
    }

    LOG( INFO, "OpenCL device: " + QString( (char *)vendor_name ) + " " + QString( (char *)device_name ) );
    qDebug() << "DSP: Found OpenCL Device " <<  QString( (char *)vendor_name ) + " " + QString( (char *)device_name );

    cl_Context = clCreateContext( 0, 1, &cl_ComputeDeviceId, NULL, NULL, &err );
    if( !cl_Context )
    {
        qDebug() << "DSP: OpenCL could not create compute context.";
        displayFailureMessage( tr( "Could not allocate OpenCL compute context, reason %1" ).arg( err ), true );
        return false;
    }

    cl_Commands = clCreateCommandQueue( cl_Context, cl_ComputeDeviceId, 0, &err );
    if( !cl_Commands )
    {
        qDebug() << "DSP: OpenCL could not create command queue.";
        displayFailureMessage( tr( "Could not create OpenCL command queue, reason %1" ).arg( err ), true );
        return false;
    }

    if( !buildOpenCLKernel( QString( path + RESCALE_CL ), "rescale_kernel", &cl_RescaleProgram, &cl_RescaleKernel ) )
    {
        return false;
    }

    if( !buildOpenCLKernel( QString( path + POSTPROC_CL ), "postproc_kernel", &cl_PostProcProgram, &cl_PostProcKernel ) )
    {
        return false;
    }

    if( !buildOpenCLKernel( QString( path + BANDC_CL ), "bandc_kernel", &cl_BandCProgram, &cl_BandCKernel ) )
    {
        return false;
    }

    if( !buildOpenCLKernel( QString( path + WARP_CL ), "warp_kernel", &cl_WarpProgram, &cl_WarpKernel ) )
    {
        return false;
    }

    if( !initOpenCLFFT() )
    {
        qDebug() << "DSP: AMDOpenCLFFT setup failed.";
        return false;
    }

    createCLMemObjects( cl_Context );

    global_unit_dim[ 0 ] = RescalingDataLength;
    global_unit_dim[ 1 ] = linesPerFrame;

    qDebug() << "DSPGPU: OpenCL init complete.";

    return true;
}

/*
 * loadCLProgramSourceFromFile
 */
char *DSPGPU::loadCLProgramSourceFromFile( QString filename )
{
    QFile     sourceFile( filename );
    QFileInfo sourceFileInfo( filename );

    char *sourceBuf;

    if( !sourceFile.open( QIODevice::ReadOnly ) )
    {
        displayFailureMessage( tr( "Failed to load OpenCL source file %1 ").arg( filename ), true );
        return NULL;
    }

    int srcSize = sourceFileInfo.size();

    // memory is freed by the calling routine
    sourceBuf = (char *)malloc( srcSize + 1 );
    sourceFile.read( sourceBuf, srcSize );
    sourceBuf[ srcSize ] = '\0'; // Very important!
    return sourceBuf;
}

/*
 * loadCLProgramBinaryFromFile
 */
QByteArray DSPGPU::loadCLProgramBinaryFromFile( QString filename )
{
    QFile objectFile( filename );

    if( !objectFile.open( QIODevice::ReadOnly ) )
    {
        displayFailureMessage( tr( "Failed to load OpenCL object file %1 ").arg( filename ), true );
        return NULL;
    }

    QByteArray objectBytes;
    objectBytes = objectFile.readAll();

    return objectBytes;
}

/*
 * createCLMemObjects
 */
bool DSPGPU::createCLMemObjects( cl_context context )
{
    int err;

    rescaleInputMemObjSize = linesPerFrame * recordLength * sizeof(unsigned short);
    rescaleInputMemObj        = clCreateBuffer( context, CL_MEM_READ_ONLY, rescaleInputMemObjSize, nullptr, nullptr );
    LOG2(rescaleInputMemObj, rescaleInputMemObjSize);

    rescaleOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    rescaleOutputMemObj       = clCreateBuffer( context, CL_MEM_READ_WRITE, rescaleOutputMemObjSize, nullptr, nullptr );
    LOG2(rescaleOutputMemObj, rescaleOutputMemObjSize);

    rescaleFracSamplesMemObjSize = RescalingDataLength * sizeof(float);
    rescaleFracSamplesMemObj  = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, rescaleFracSamplesMemObjSize, fractionalSamples, nullptr );
    LOG2(rescaleFracSamplesMemObj, rescaleFracSamplesMemObjSize);

    rescaleWholeSamplesMemObjSize = RescalingDataLength * sizeof(float);
    rescaleWholeSamplesMemObj = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, rescaleWholeSamplesMemObjSize, wholeSamples, nullptr );
    LOG2(rescaleWholeSamplesMemObj, rescaleWholeSamplesMemObjSize);

    fftImaginaryInputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    fftImaginaryInputMemObj   = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, fftImaginaryInputMemObjSize, fftImaginaryBuffer, nullptr );
    LOG2(fftImaginaryInputMemObj, fftImaginaryInputMemObjSize);

    postProcOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(unsigned short);
    postProcOutputMemObj      = clCreateBuffer( context, CL_MEM_WRITE_ONLY, postProcOutputMemObjSize, nullptr, nullptr );
    LOG2(postProcOutputMemObj, postProcOutputMemObjSize);

    windowMemObjSize = RescalingDataLength * sizeof(float);
    windowMemObj              = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, windowMemObjSize, windowBuffer, nullptr );
    LOG2(windowMemObj, windowMemObjSize);

    fftRealOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    fftRealOutputMemObj       = clCreateBuffer( context, CL_MEM_READ_WRITE, fftRealOutputMemObjSize, nullptr, nullptr );
    LOG2(fftRealOutputMemObj, fftRealOutputMemObjSize);

    fftImaginaryOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    fftImaginaryOutputMemObj  = clCreateBuffer( context, CL_MEM_READ_WRITE, fftImaginaryOutputMemObjSize, nullptr, nullptr );
    LOG2(fftImaginaryOutputMemObj, fftImaginaryOutputMemObjSize);

    lastFramePreScalingMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    lastFramePreScalingMemObj = clCreateBuffer( context, CL_MEM_READ_WRITE, lastFramePreScalingMemObjSize, nullptr, nullptr );
    LOG2(lastFramePreScalingMemObj, lastFramePreScalingMemObjSize);

    cl_image_format clImageFormat;
    clImageFormat.image_channel_order     = CL_R;
    clImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;

    inputImageMemObj       = clCreateImage2D( context, CL_MEM_READ_WRITE, &clImageFormat, MaxALineLength, linesPerFrame, 0, nullptr, &err );

    warpInputImageMemObj   = clCreateImage2D( context, CL_MEM_READ_WRITE, &clImageFormat, MaxALineLength, linesPerFrame, 0, nullptr, &err );

    outputImageMemObj      = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &clImageFormat, SectorWidth_px, SectorHeight_px, 0, nullptr, &err );

    outputVideoImageMemObj = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &clImageFormat, SectorWidth_px, SectorHeight_px, 0, nullptr, &err );

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Failed to create GPU images" ), true );
        return false;
    }

    return true;
}

/*
 * transformData
 *
 * FFT the input laser signal, post process to 8-bit and warp to sector.
 */
bool DSPGPU::transformData( unsigned char *dispData, unsigned char *videoData )
{
   clAmdFftStatus fftStatus;
   int            clStatus;
   int            averageVal   = (int)doAveraging;
   int            invertColors = (int)doInvertColors;

   cl_mem         inputMemObjects[ 2 ]  = { rescaleOutputMemObj, fftImaginaryInputMemObj };
   cl_mem         outputMemObjects[ 2 ] = { fftRealOutputMemObj, fftImaginaryOutputMemObj };

   // XXX: Empirically set to achieve full range at just below detector saturation
   // scaleFactor adjusted for new DAQ Input Range for HS devices. See #1777, #1769
   float scaleFactor = (float) ( ( 20000.0 * 255.0 ) / 65535.0 );
   const unsigned int dcNoiseLevel = 150.0f; // XXX: Empirically measured

   clStatus = clEnqueueReadBuffer( cl_Commands,               // command_queue
                                   fftImaginaryOutputMemObj,  // buffer
                                   false,                     // blocking_read
                                   0,                         // offset
                                   sizeof( float ) * 1024,    // number of bytes to read
                                   imData,                    // ptr
                                   0,                         // set to zero
                                   NULL,                      // goes with above zero
                                   NULL );                    // see page 261 of OpenCL Programming Guide

   clStatus |= clEnqueueReadBuffer( cl_Commands,               // command_queue
                                    fftRealOutputMemObj,       // buffer
                                    false,                     // blocking_read
                                    0,                         // offset
                                    sizeof( float ) * 1024,    // number of bytes to read
                                    reData,                    // ptr
                                    0,                         // set to zero
                                    NULL,                      // goes with above zero
                                    NULL );                    // see page 261 of OpenCL Programming Guide

   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to enqueue commands to read from buffer objects: "  << clStatus;
       return false;
   }

   // pull out one A-line for the FFT display
   pData->fftData[ 0 ] = 0;
   for( int i = 1; i < 1024; i++ )
   {
       pData->fftData[ i ] = (unsigned short)( scaleFactor * log10( sqrt( ( imData[i] * imData[i] ) + ( reData[i] * reData[i] ) ) ) - dcNoiseLevel );

       // set out-of-range values to zero (only for the graph)
       if( pData->fftData[ i ] < 0 || pData->fftData[ i ] > 255 )
       {
           pData->fftData[ i ] = 0;
       }
   }

   fftStatus = clAmdFftEnqueueTransform( hCl_fft_plan, CLFFT_FORWARD, 1, &cl_Commands, 0,
                                         NULL, NULL, inputMemObjects, outputMemObjects, NULL );

   if( fftStatus != CLFFT_SUCCESS ) {
       qDebug() << "DSP: Failed to enqueue fft operation to OpenCL command queue.";
   }

   unsigned int inputLength = RescalingDataLength;

   // Make this a loop XXX
   clStatus  = clSetKernelArg( cl_PostProcKernel, 0, sizeof(cl_mem), &fftRealOutputMemObj );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 0 , err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 1, sizeof(cl_mem), &fftImaginaryOutputMemObj );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 1, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 2, sizeof(cl_mem), &lastFramePreScalingMemObj );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 2, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 3, sizeof(cl_mem), &inputImageMemObj );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 3, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 4, sizeof(int), &inputLength );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 4, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 5, sizeof(float), &scaleFactor );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 5, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 6, sizeof(unsigned int), &dcNoiseLevel );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 6, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 7, sizeof(int), &averageVal );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 7, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 8, sizeof(float), &prevFrameWeight_percent );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 8, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 9, sizeof(float), &currFrameWeight_percent );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 9, err: "  << clStatus;
   }
   clStatus |= clSetKernelArg( cl_PostProcKernel, 10, sizeof(int), &invertColors );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set post processing argument 10, err: "  << clStatus;
   }

   global_unit_dim[ 0 ] = FFTDataSize;
   global_unit_dim[ 1 ] = linesPerFrame; // Operate on 1/2 of 1/2 of FFT data (3mm depth). How to change this cleanly? Make a post-proc global dim. XXX
   clStatus = clEnqueueNDRangeKernel( cl_Commands, cl_PostProcKernel, 2, NULL, global_unit_dim, local_unit_dim, 0, NULL, NULL );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to execute post-processing kernel, reason: " << clStatus;
       return false;
   }

    /*
     * Adjust brightness and contrast
     */
    clStatus  = clSetKernelArg( cl_BandCKernel, 0, sizeof(cl_mem), &inputImageMemObj );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 0 , err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( cl_BandCKernel, 1, sizeof(cl_mem), &warpInputImageMemObj );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 1, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( cl_BandCKernel, 2, sizeof(float), &blackLevel );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 2, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( cl_BandCKernel, 3, sizeof(float), &whiteLevel );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 3, err: "  << clStatus;
    }
    global_unit_dim[ 0 ] = FFTDataSize;
    global_unit_dim[ 1 ] = linesPerFrame; // Operate on 1/2 of 1/2 of FFT data (3mm depth). How to change this cleanly? Make a post-proc global dim. XXX
    clStatus = clEnqueueNDRangeKernel( cl_Commands, cl_BandCKernel, 2, NULL, global_unit_dim, local_unit_dim, 0, NULL, NULL );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to execute B and C kernel, reason: " << clStatus;
        return false;
    }

   // Set true by default, means the sector draws Counter-Clockwise. This variable is necessary because we pass an address as an argument.
   int reverseDirection = (int)useDistalToProximalView;

   // get variables and pass into Warp.CL
   depthSetting &depth = depthSetting::Instance();
   const int   imagingDepth_S   = depth.getDepth_S();
   const float fractionOfCanvas = depth.getFractionOfCanvas();

   deviceSettings &dev = deviceSettings::Instance();
   const float standardDepth_mm = dev.current()->getImagingDepthNormal_mm();
   const int   standardDepth_S  = dev.current()->getALineLengthNormal_px();
   reverseDirection ^= (int)dev.current()->getRotation();    // apply Sled rotational direction

   clStatus  = clSetKernelArg( cl_WarpKernel,  0, sizeof(cl_mem), &warpInputImageMemObj );
   clStatus |= clSetKernelArg( cl_WarpKernel,  1, sizeof(cl_mem), &outputImageMemObj );
   clStatus |= clSetKernelArg( cl_WarpKernel,  2, sizeof(cl_mem), &outputVideoImageMemObj );
   clStatus |= clSetKernelArg( cl_WarpKernel,  3, sizeof(float),  &catheterRadius_um );
   clStatus |= clSetKernelArg( cl_WarpKernel,  4, sizeof(float),  &internalImagingMask_px );
   clStatus |= clSetKernelArg( cl_WarpKernel,  5, sizeof(float),  &standardDepth_mm );
   clStatus |= clSetKernelArg( cl_WarpKernel,  6, sizeof(int),    &standardDepth_S );
   clStatus |= clSetKernelArg( cl_WarpKernel,  7, sizeof(float),  &displayAngle_deg );
   clStatus |= clSetKernelArg( cl_WarpKernel,  8, sizeof(int),    &reverseDirection );
   clStatus |= clSetKernelArg( cl_WarpKernel,  9, sizeof(int),    &SectorWidth_px );
   clStatus |= clSetKernelArg( cl_WarpKernel, 10, sizeof(int),    &SectorHeight_px );
   clStatus |= clSetKernelArg( cl_WarpKernel, 11, sizeof(float),  &fractionOfCanvas );
   clStatus |= clSetKernelArg( cl_WarpKernel, 12, sizeof(int),    &imagingDepth_S );

   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to set warp kernel arguments:" << clStatus;
       return false;
   }

   global_unit_dim[ 0 ] = SectorWidth_px;
   global_unit_dim[ 1 ] = SectorHeight_px;

   clStatus = clEnqueueNDRangeKernel( cl_Commands, cl_WarpKernel, 2, NULL, global_unit_dim, local_unit_dim, 0, NULL, NULL );

   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to execute warp kernel:" << clStatus;
       return false;
   }
   global_unit_dim[ 0 ] = RescalingDataLength;
   global_unit_dim[ 1 ] = linesPerFrame;

   // Do all the work that was queued up on the GPU
   clStatus = clFinish( cl_Commands );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to issue and complete all queued commands: " << clStatus;
       return false;
   }

   size_t origin[ 3 ] = { 0, 0, 0 };
   size_t region[ 3 ] = { SectorWidth_px, SectorHeight_px, 1 };

   /*
    * read out the display frame
    */
   clStatus = clEnqueueReadImage( cl_Commands, outputImageMemObj, CL_TRUE, origin, region, 0, 0, dispData, 0, NULL, NULL );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to read back final image data from warp kernel: " << clStatus;
       return false;
   }

   /*
    * read out the video frame
    */
   clStatus = clEnqueueReadImage( cl_Commands, outputVideoImageMemObj, CL_TRUE, origin, region, 0, 0, videoData, 0, NULL, NULL );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to read back video image data from warp kernel: " << clStatus;
       return false;
   }

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
 */
unsigned int DSPGPU::rescale( const unsigned short *inputData )
{
    TIME_THIS_SCOPE( dsp_rescale );
    rescaleInputMemObjSize = linesPerFrame * recordLength * sizeof(unsigned short);
    int err = clEnqueueWriteBuffer( cl_Commands,
                                    rescaleInputMemObj,
                                    true,
                                    0,
                                    rescaleInputMemObjSize,
                                    inputData,
                                    0,
                                    nullptr,
                                    nullptr );
    LOG2(rescaleInputMemObj, rescaleInputMemObjSize)

    if( err != CL_SUCCESS )
    {
        qDebug() << "Error: Failed to enqueue new data to GPU! Err = " << err;
        return 1;
    }
    err  = clSetKernelArg( cl_RescaleKernel, 0, sizeof(cl_mem),       &rescaleInputMemObj );
    err |= clSetKernelArg( cl_RescaleKernel, 1, sizeof(cl_mem),       &rescaleOutputMemObj );
    err |= clSetKernelArg( cl_RescaleKernel, 2, sizeof(cl_mem),       &rescaleFracSamplesMemObj );
    err |= clSetKernelArg( cl_RescaleKernel, 3, sizeof(cl_mem),       &rescaleWholeSamplesMemObj );
    err |= clSetKernelArg( cl_RescaleKernel, 4, sizeof(cl_mem),       &windowMemObj );
    err |= clSetKernelArg( cl_RescaleKernel, 5, sizeof(unsigned int), &recordLength );
    err |= clSetKernelArg( cl_RescaleKernel, 6, sizeof(unsigned int), &RescalingDataLength );

    err |= clEnqueueNDRangeKernel( cl_Commands, cl_RescaleKernel, 2, NULL, global_unit_dim, local_unit_dim, 0, NULL, NULL );

    if( err != CL_SUCCESS )
    {
        qDebug() << "Error: Failed to execute kernel! Err = " << err;
        qDebug() << "global_unit_dim[0] = " << global_unit_dim[ 0 ] << "global_unit_dim[1] = " << global_unit_dim[ 1 ] << "\n"
                 << "local_unit_dim[0]  = " << local_unit_dim[ 0 ]  << "local_unit_dim[1]  = " << local_unit_dim[ 1 ];
        return 1;
    }
    auto tgi = TheGlobals::instance();
    LOG3(local_unit_dim[ 0 ], local_unit_dim[ 1 ], tgi->getGFrameCounter());
    LOG3(global_unit_dim[ 0 ], global_unit_dim[ 1 ], tgi->getGDaqRawData_idx());

    return 0;
}
