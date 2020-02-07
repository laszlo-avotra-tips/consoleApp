/*
 * dspgpu.cpp
 *
 * Handle the fft signal data from the DAQ
 *    - Post process
 *    - Transform to polar
 *    - Scale output for display
 *
 * The input data is expected to be in two arrays of float, the first
 *
 * Author(s): Chris White, Dennis W. Jackson, Ryan Radjabi modified by lcv
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

#include "defaults.h"
#include "dspgpu.h"
#include "logger.h"
#include "profiler.h"
#include "util.h"
#include "buildflags.h"
#include "deviceSettings.h"
#include "Backend/depthsetting.h"
//#include "ipp.h"
#include "theglobals.h"
#include "signalmanager.h"
#include "playbackmanager.h"


#if _DEBUG
#   define POSTPROC_CL "./backend/opencl/postproc.cl"
#   define BANDC_CL    "./backend/opencl/bandc.cl"
#   define WARP_CL     "./backend/opencl/warp.cl"
#else
#   define POSTPROC_CL "./postproc.cl"
#   define BANDC_CL    "./bandc.cl"
#   define WARP_CL     "./warp.cl"
#endif

/*
 * OpenCL core organization parameters
 */
#define DEFAULT_GLOBAL_UNITS ( 2048 )
#define DEFAULT_LOCAL_UNITS  ( 16 )

namespace{
size_t global_unit_dim[] = { DEFAULT_GLOBAL_UNITS, DEFAULT_GLOBAL_UNITS };
size_t local_unit_dim[]  = { DEFAULT_LOCAL_UNITS,  DEFAULT_LOCAL_UNITS  };
}

/*
 * destructor
 *
 * The DSP base constructor is called implicitly
 */
DSPGPU::~DSPGPU()
{
    LOG( INFO, "DSP GPU shutdown" )
    qDebug() << "DSPGPU::~DSPGPU()";

    stop();
    wait( 100 ); //ms

    /*
     * Clean up openCL objects
     */
    for(auto oclfp : m_openClFunctionMap){
        auto oclf = oclfp.second;
        clReleaseProgram(oclf.first);
        clReleaseKernel(oclf.second);
    }

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
void DSPGPU::init( size_t inputLength,
                   size_t frameLines,
                   size_t inBytesPerRecord,
                   size_t inBytesPerBuffer)
{
    // call the common initilization steps
    DSP::init( inputLength, frameLines, inBytesPerRecord, inBytesPerBuffer );

    // XXX Need to pass these into the DSP so we dont copy buffers there
    // for opencl
    try {
        workingBuffer = std::make_unique<quint16 []>(bytesPerBuffer / sizeof (quint16));
        imData = std::make_unique<float []>(complexDataSize);
        reData = std::make_unique<float []>(complexDataSize);
    } catch (std::bad_alloc e) {
        emit sendError( e.what() );
    }

    // Configure the DSP
    if( !initOpenCL() )
    {
        displayFailureMessage( tr( "Could not initialize OpenCL." ), true );
    }

    doAveraging      = false;
    displayAngle_deg = 0.0;

    currFrameWeight_percent = DefaultCurrFrameWeight_Percent / 100.0;
    prevFrameWeight_percent = 1 - currFrameWeight_percent;
}

bool DSPGPU::processData(int index)
{
    bool success(false);

    auto pmi = PlaybackManager::instance();
    if(pmi->isFrameQueue() && pmi->findDisplayBuffer(index, pData)){
        if( !transformData( pData->dispData, pData->videoData ) )   //Success if return true
        {
            LOG( WARNING, "Failed to transform data on GPU." )
        }

        /*
         * Frame counts are filled in by the Data Consumer;
         * this keeps the frame counts continuous.
         */
        pData->frameCount      = 0;
        pData->encoderPosition = 0; // NOT USED for fast-OCT
        pData->timeStamp       = getTimeStamp();
        pData->milliseconds    = quint16(getMilliseconds());

        success = true;

        PlaybackManager::instance()->frameReady(index);
    } else{
        LOG1(pData)
    }
    return success;
}

QString DSPGPU::clCreateBufferErrorVerbose(int clError) const
{
    QString cause;
    switch(clError){
        case CL_INVALID_CONTEXT :       {cause = "CL_INVALID_CONTEXT";} break;
        case CL_INVALID_VALUE :         {cause = "CL_INVALID_VALUE";} break;
        case CL_INVALID_BUFFER_SIZE :   {cause = "CL_INVALID_BUFFER_SIZE";} break;
        case CL_INVALID_HOST_PTR :      {cause = "CL_INVALID_HOST_PTR";} break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE :  {cause = "CL_MEM_OBJECT_ALLOCATION_FAILURE";} break;
        case CL_OUT_OF_HOST_MEMORY :    {cause = "CL_OUT_OF_HOST_MEMORY";} break;
    }
    return cause;
}

bool DSPGPU::isClReturnValueSuccess(cl_int ret, int line) const
{
    QString errorMesage;
    QTextStream qts(&errorMesage);

    bool success{true};
    if(ret != CL_SUCCESS){
        success = false;
        qts << " Open CL failes at line " << line;
        LOG1(errorMesage)
    }
    return success;
}

void DSPGPU::initOpenClFileMap()
{
#ifdef QT_DEBUG
    QString path = QCoreApplication::applicationDirPath() + QString("/../../consoleApp/");
#else
    QString path = QCoreApplication::applicationDirPath();
#endif

    m_openClFileMap = {
        {"postproc_kernel",  path + POSTPROC_CL},
        {"bandc_kernel",  path + BANDC_CL},
        {"warp_kernel",  path + WARP_CL}
    };
}

cl_platform_id DSPGPU::getPlatformId() const
{
    cl_platform_id platformId;
    cl_uint        numPlatforms = 0;

    cl_int err = clGetPlatformIDs( 0, nullptr, &numPlatforms );
    qDebug() << "numPlatforms =" << numPlatforms;

    if( numPlatforms == 0 )
    {
        // fatal error
        displayFailureMessage( tr( "Could not find openCL platform, reason: %1" ).arg( err ), true );
        return nullptr;
    }

    // Found openCL-capable platforms
    cl_platform_id* platformIds = new cl_platform_id [numPlatforms];
    err = clGetPlatformIDs( numPlatforms, platformIds, nullptr );

    uint deviceIndex = 99;

    const int DefaultStringSize = 128;
    char vendor[ DefaultStringSize ];
    char name[ DefaultStringSize ];
    char version[ DefaultStringSize ];

    for ( cl_uint i = 0; i < numPlatforms; i++ )
    {
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VENDOR,  DefaultStringSize, vendor,  nullptr );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_NAME,    DefaultStringSize, name,    nullptr );
        err |= clGetPlatformInfo( platformIds[ i ], CL_PLATFORM_VERSION, DefaultStringSize, version, nullptr );

        if ( err != CL_SUCCESS )
        {
            displayFailureMessage( tr( "Could not enumerate OpenCL platform IDs, reason: %1" ).arg( err ), true );
            return nullptr;
        }
        qDebug() << "Platform (" << i << ") Vendor:" << vendor << " Name:" << name << " Version:" << version;

        if ( QString( vendor ) == "Intel(R) Corporation" &&
             QString( name )   == "Intel(R) OpenCL" )
        {
            deviceIndex = i;
        }
    }

    if ( deviceIndex > 10 )
    {
        displayFailureMessage( tr( "Could not find gpu" ), true );
        return nullptr;
    }

    platformId = platformIds[ deviceIndex ];

    // release the memory.  Error paths do not free the memory since they will shut down the program
    delete [] platformIds;

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Could not enumerate OpenCL platform IDs, reason: %1" ).arg( err ), true );
        return nullptr;
    }

    return platformId;
}

bool DSPGPU::getGpuDeviceInfo(cl_platform_id id, bool isLogging)
{
    // Verify the GPU is present
    cl_int err = clGetDeviceIDs( id, CL_DEVICE_TYPE_GPU, 1, &cl_ComputeDeviceId, nullptr );

    // If not, fall back to the CPU. Display a warning if this occurs on the release hardware
    if( err == CL_DEVICE_NOT_FOUND )
    {
        // fall back to CPU when debugging if GPU not present
        err = clGetDeviceIDs( id, CL_DEVICE_TYPE_CPU, 1, &cl_ComputeDeviceId, nullptr );
    }

    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Could not get OpenCL device IDs, reason: %1" ).arg( err ), true );
        return false;
    }

    size_t returned_size( 0 );
    cl_uint maxComputeUnits;
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }
    if(isLogging) LOG2(maxComputeUnits,returned_size)

    cl_uint maxWorkItemDimentions;
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxWorkItemDimentions), &maxWorkItemDimentions, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }
    if(isLogging) LOG2(maxWorkItemDimentions,returned_size)

    size_t maxWorkItemSizes[3];
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }
    if(isLogging) LOG3(maxWorkItemSizes[0],maxWorkItemSizes[1],maxWorkItemSizes[2])

    err = clGetDeviceInfo( cl_ComputeDeviceId,
                           CL_DEVICE_MAX_WORK_GROUP_SIZE,
                           sizeof( cl_max_workgroup_size ),
                           &cl_max_workgroup_size,
                           &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        return false;
    }


    size_t maxWorkGroupSize;
    err = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, &returned_size );
    if(!isClReturnValueSuccess(err,__LINE__)){
        displayFailureMessage( tr( "Could not enumerate OpenCL device IDs, reason: %1" ).arg( err ), true );
        return false;
    }
    if(isLogging) LOG2(maxWorkGroupSize,returned_size)

    cl_char vendor_name[ 1024 ] = { 0 };
    cl_char device_name[ 1024 ] = { 0 };
    err  = clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_VENDOR, sizeof( vendor_name ), vendor_name, &returned_size);
    err |= clGetDeviceInfo( cl_ComputeDeviceId, CL_DEVICE_NAME, sizeof( device_name ), device_name, &returned_size);

    if(!isClReturnValueSuccess(err,__LINE__)){
        displayFailureMessage( tr( "Could not get OpenCL device info, reason: %1").arg( err ), true );
        return false;
    }
    LOG( INFO, "OpenCL device: " + QString( reinterpret_cast<char*>(vendor_name)) + " " + QString( reinterpret_cast<char*>(device_name)) )
    if(isLogging) qDebug() << "DSP: Found OpenCL Device " <<  QString( reinterpret_cast<char*>(vendor_name) ) + " " + QString( reinterpret_cast<char*>(device_name) );

    return true;
}

/*
 * buildOpenCLKernel
 *
 * Load, compile and build the program for the given OpenCL kernel.
 */
bool DSPGPU::buildOpenCLKernel( QString clSourceFile, const char *kernelName, cl_program *program, cl_kernel *kernel )
{
//    qDebug() << "DSPGPU::buildOpenCLKernel:" << clSourceFile;
    QTime buildTimer;
    buildTimer.start();

    int err;

    /*
     * Load, compile, link the source
     */
    LOG1(kernelName)
    const char *sourceBuf = loadCLProgramSourceFromFile( clSourceFile ); // XXX: We should switch to pre-compiled binary. See #1057
    if( !sourceBuf )
    {
        displayFailureMessage( tr( "Failed to load program source file %1 (%2)" ).arg( clSourceFile ).arg( QDir::currentPath() ), true );
        return false;
    }

    /*
     * Create the compute program(s) from the source buffer
     */
    *program = clCreateProgramWithSource( cl_Context, 1, &sourceBuf, nullptr, &err );
    if( !*program || ( err != CL_SUCCESS ) )
    {
        qDebug() << "DSP: OpenCL could not create program from source: " << err;
        displayFailureMessage( tr( "Could not build OpenCL kernel from source, reason %1" ).arg( err ), true );
        return false;
    }
    delete [] sourceBuf;

    err = clBuildProgram( *program, 0, nullptr, nullptr, nullptr, nullptr);
    if( err != CL_SUCCESS )
    {
        size_t length;
        const int BuildLogLength = 2048;
        char *build_log = new char [BuildLogLength];

        qDebug() << "DSP: OpenCL build failed: " << err;
        clGetProgramBuildInfo( *program, cl_ComputeDeviceId, CL_PROGRAM_BUILD_LOG, BuildLogLength, build_log, &length );
        qDebug() << "openCl Build log:" << build_log;

        displayFailureMessage( tr( "Could not build program, reason %1" ).arg( err ), true );
        delete []  build_log;
        return false;
    }

    *kernel = clCreateKernel( *program, kernelName, &err );

    if( err != CL_SUCCESS )
    {
        qDebug() << "DSP: OpenCL could not create compute kernel: " << err;
        displayFailureMessage( tr( "Could not create compute kernel, reason %1" ).arg( err ), true );
        return false;
    }
//    LOG1( buildTimer.elapsed());
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


    initOpenClFileMap();

    cl_platform_id platformId = getPlatformId();

    if(!platformId){
        return false;
    }

    const bool logInfo{false};
    bool success = getGpuDeviceInfo(platformId, logInfo);

    if(!success){
        return false;
    }
    cl_int err;
    cl_Context = clCreateContext( nullptr, 1, &cl_ComputeDeviceId, nullptr, nullptr, &err );
    if( !cl_Context )
    {
        qDebug() << "DSP: OpenCL could not create compute context.";
        displayFailureMessage( tr( "Could not allocate OpenCL compute context, reason %1" ).arg( err ), true );
        return false;
    }

    cl_Commands = clCreateCommandQueueWithProperties( cl_Context, cl_ComputeDeviceId, nullptr, &err );
    if( !cl_Commands )
    {
        qDebug() << "DSP: OpenCL could not create command queue.";
        displayFailureMessage( tr( "Could not create OpenCL command queue, reason %1" ).arg( err ), true );
        return false;
    }

    for( const auto& sourceCode : m_openClFileMap){
        const auto& kernelFunction = sourceCode.first;
        auto it = m_openClFunctionMap.find(kernelFunction);
        if(it != m_openClFunctionMap.end()){
            bool success = buildOpenCLKernel(sourceCode.second, sourceCode.first.toLatin1(),
                                             &it->second.first, &it->second.second);
            if(!success){
                return false;
            }
        }
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
        return nullptr;
    }

    auto srcSize = sourceFileInfo.size();

    // memory is freed by the calling routine
    sourceBuf = new char [size_t(srcSize + 1) ];
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
        return nullptr;
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
    cl_int err;

    fftRealOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    fftImaginaryOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);

    if(SignalManager::instance()->isFftSource()){
        fftRealOutputMemObj       =
                clCreateBuffer( context, CL_MEM_READ_WRITE, fftRealOutputMemObjSize, nullptr , &err );

        if( err != CL_SUCCESS )
        {
             displayFailureMessage( tr( "Failed to create fftRealOutputMemObj" ), true );
            return false;
        }

        fftImaginaryOutputMemObj  =
                clCreateBuffer( context, CL_MEM_READ_WRITE, fftImaginaryOutputMemObjSize, nullptr , &err );
        if( err != CL_SUCCESS )
        {
             displayFailureMessage( tr( "Failed to create fftImaginaryOutputMemObj" ), true );
             return false;
        }

    }else{
        fftRealOutputMemObj       = clCreateBuffer( context, CL_MEM_READ_WRITE, fftRealOutputMemObjSize, nullptr, nullptr );

        fftImaginaryOutputMemObj  = clCreateBuffer( context, CL_MEM_READ_WRITE, fftImaginaryOutputMemObjSize, nullptr, nullptr );

    }

    lastFramePreScalingMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);

    if(SignalManager::instance()->isPreScalingSource()){
        lastFramePreScalingMemObj
                = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, lastFramePreScalingMemObjSize, SignalManager::instance()->getLastFramePrescaling(), &err );

    } else {
        lastFramePreScalingMemObj = clCreateBuffer( context, CL_MEM_READ_WRITE, lastFramePreScalingMemObjSize, nullptr, &err );
    }
    if( err != CL_SUCCESS )
    {
         displayFailureMessage( tr( "Failed to create lastFramePreScalingMemObj" ), true );
         return false;
    }

    const cl_image_format clImageFormat{CL_R,CL_UNSIGNED_INT8};

    const cl_mem_object_type image_type{CL_MEM_OBJECT_IMAGE2D};

    const size_t input_image_width{MaxALineLength};
    const size_t output_image_width{SectorWidth_px};

    const size_t input_image_height{linesPerFrame};
    const size_t output_image_height{SectorHeight_px};

    const size_t image_depth{1};
    const size_t image_array_size{1};
    const size_t image_row_pitch{0};
    const size_t image_slice_pitch{0};
    const cl_uint num_mip_levels{0};
    const cl_uint num_samples{0};
    cl_mem buffer{nullptr};

    const cl_image_desc inputImageDescriptor{
        image_type,
        input_image_width,
        input_image_height,
        image_depth,
        image_array_size,
        image_row_pitch,
        image_slice_pitch,
        num_mip_levels,
        num_samples,
        {buffer}
    };

    const cl_image_desc outputImageDescriptor{
        image_type,
        output_image_width,
        output_image_height,
        image_depth,
        image_array_size,
        image_row_pitch,
        image_slice_pitch,
        num_mip_levels,
        num_samples,
        {buffer}
    };


    inputImageMemObj       = clCreateImage ( context, CL_MEM_READ_WRITE, &clImageFormat, &inputImageDescriptor, nullptr, &err );
    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Failed to create GPU images" ), true );
        return false;
    }

    warpInputImageMemObj   = clCreateImage ( context, CL_MEM_READ_WRITE, &clImageFormat, &inputImageDescriptor, nullptr, &err );
    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Failed to create GPU images" ), true );
        return false;
    }

    outputImageMemObj      = clCreateImage( context, CL_MEM_WRITE_ONLY, &clImageFormat, &outputImageDescriptor, nullptr, &err );
    if( err != CL_SUCCESS )
    {
        displayFailureMessage( tr( "Failed to create GPU images" ), true );
        return false;
    }

    outputVideoImageMemObj = clCreateImage( context, CL_MEM_WRITE_ONLY, &clImageFormat, &outputImageDescriptor, nullptr, &err );
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
   int            clStatus;
   int            averageVal   = doAveraging;
   int            invertColors = doInvertColors;

   float scaleFactor = 20000.0f * 255.0f / 65535.0f ;
   const unsigned int dcNoiseLevel = 150.0f; // XXX: Empirically measured

   unsigned int postProcInputLength = RescalingDataLength;

   auto it = m_openClFunctionMap.find("postproc_kernel");
   if(it != m_openClFunctionMap.end())
   {
        auto& oclppk = it->second.second;
        // Make this a loop XXX
        clStatus  = clSetKernelArg( oclppk, 0, sizeof(cl_mem), &fftRealOutputMemObj );
        if( clStatus != CL_SUCCESS )
        {
           qDebug() << "DSP: Failed to set post processing argument 0 , err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclppk, 1, sizeof(cl_mem), &fftImaginaryOutputMemObj );
        if( clStatus != CL_SUCCESS )
        {
           qDebug() << "DSP: Failed to set post processing argument 1, err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclppk, 2, sizeof(cl_mem), &lastFramePreScalingMemObj );
        if( clStatus != CL_SUCCESS )
        {
           qDebug() << "DSP: Failed to set post processing argument 2, err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclppk, 3, sizeof(cl_mem), &inputImageMemObj );
        if( clStatus != CL_SUCCESS )
        {
           qDebug() << "DSP: Failed to set post processing argument 3, err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclppk, 4, sizeof(int), &postProcInputLength );
        if( clStatus != CL_SUCCESS )
        {
           qDebug() << "DSP: Failed to set post processing argument 4, err: "  << clStatus;
        }
       clStatus |= clSetKernelArg( oclppk, 5, sizeof(float), &scaleFactor );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set post processing argument 5, err: "  << clStatus;
       }
       clStatus |= clSetKernelArg( oclppk, 6, sizeof(unsigned int), &dcNoiseLevel );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set post processing argument 6, err: "  << clStatus;
       }
       clStatus |= clSetKernelArg( oclppk, 7, sizeof(int), &averageVal );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set post processing argument 7, err: "  << clStatus;
       }
       clStatus |= clSetKernelArg( oclppk, 8, sizeof(float), &prevFrameWeight_percent );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set post processing argument 8, err: "  << clStatus;
       }
       clStatus |= clSetKernelArg( oclppk, 9, sizeof(float), &currFrameWeight_percent );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set post processing argument 9, err: "  << clStatus;
       }
       clStatus |= clSetKernelArg( oclppk, 10, sizeof(int), &invertColors );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set post processing argument 10, err: "  << clStatus;
       }

       global_unit_dim[ 0 ] = FFTDataSize;
       global_unit_dim[ 1 ] = linesPerFrame; // Operate on 1/2 of 1/2 of FFT data (3mm depth). How to change this cleanly? Make a post-proc global dim. XXX
       clStatus = clEnqueueNDRangeKernel( cl_Commands, oclppk, 2, nullptr, global_unit_dim, local_unit_dim, 0, nullptr, nullptr );
       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to execute post-processing kernel, reason: " << clStatus;
           return false;
       }
   }

    /*
     * Adjust brightness and contrast
     */
   auto itbc = m_openClFunctionMap.find("bandc_kernel");
   if(itbc != m_openClFunctionMap.end())
   {
        auto& oclbck = itbc->second.second;
        clStatus  = clSetKernelArg( oclbck, 0, sizeof(cl_mem), &inputImageMemObj );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to set B and C argument 0 , err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclbck, 1, sizeof(cl_mem), &warpInputImageMemObj );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to set B and C argument 1, err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclbck, 2, sizeof(float), &blackLevel );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to set B and C argument 2, err: "  << clStatus;
        }
        clStatus |= clSetKernelArg( oclbck, 3, sizeof(float), &whiteLevel );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to set B and C argument 3, err: "  << clStatus;
        }
        global_unit_dim[ 0 ] = FFTDataSize;
        global_unit_dim[ 1 ] = linesPerFrame; // Operate on 1/2 of 1/2 of FFT data (3mm depth). How to change this cleanly? Make a post-proc global dim. XXX
        clStatus = clEnqueueNDRangeKernel( cl_Commands, oclbck, 2, nullptr, global_unit_dim, local_unit_dim, 0, nullptr, nullptr );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to execute B and C kernel, reason: " << clStatus;
            return false;
        }
   }

   // Set true by default, means the sector draws Counter-Clockwise. This variable is necessary because we pass an address as an argument.
   int reverseDirection = useDistalToProximalView;

   // get variables and pass into Warp.CL
   depthSetting &depth = depthSetting::Instance();
   const int   imagingDepth_S   = int(depth.getDepth_S());
   const float fractionOfCanvas = depth.getFractionOfCanvas();

   deviceSettings &dev = deviceSettings::Instance();
   const float standardDepth_mm = dev.current()->getImagingDepthNormal_mm();
   const int   standardDepth_S  = dev.current()->getALineLengthNormal_px();
   reverseDirection ^= int(dev.current()->getRotation());    // apply Sled rotational direction

   auto itw = m_openClFunctionMap.find("warp_kernel");
   if(itw != m_openClFunctionMap.end())
   {
       auto& oclwk = itw->second.second;
       clStatus  = clSetKernelArg( oclwk,  0, sizeof(cl_mem), &warpInputImageMemObj );
       clStatus |= clSetKernelArg( oclwk,  1, sizeof(cl_mem), &outputImageMemObj );
       clStatus |= clSetKernelArg( oclwk,  2, sizeof(cl_mem), &outputVideoImageMemObj );
       clStatus |= clSetKernelArg( oclwk,  3, sizeof(float),  &catheterRadius_um );
       clStatus |= clSetKernelArg( oclwk,  4, sizeof(float),  &internalImagingMask_px );
       clStatus |= clSetKernelArg( oclwk,  5, sizeof(float),  &standardDepth_mm );
       clStatus |= clSetKernelArg( oclwk,  6, sizeof(int),    &standardDepth_S );
       clStatus |= clSetKernelArg( oclwk,  7, sizeof(float),  &displayAngle_deg );
       clStatus |= clSetKernelArg( oclwk,  8, sizeof(int),    &reverseDirection );
       clStatus |= clSetKernelArg( oclwk,  9, sizeof(int),    &SectorWidth_px );
       clStatus |= clSetKernelArg( oclwk, 10, sizeof(int),    &SectorHeight_px );
       clStatus |= clSetKernelArg( oclwk, 11, sizeof(float),  &fractionOfCanvas );
       clStatus |= clSetKernelArg( oclwk, 12, sizeof(int),    &imagingDepth_S );

       if( clStatus != CL_SUCCESS )
       {
           qDebug() << "DSP: Failed to set warp kernel arguments:" << clStatus;
           return false;
       }

       global_unit_dim[ 0 ] = SectorWidth_px;
       global_unit_dim[ 1 ] = SectorHeight_px;

       clStatus = clEnqueueNDRangeKernel( cl_Commands, oclwk, 2, nullptr, global_unit_dim, local_unit_dim, 0, nullptr, nullptr );

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
   }

   size_t origin[ 3 ] = { 0, 0, 0 };
   size_t region[ 3 ] = { SectorWidth_px, SectorHeight_px, 1 };

   /*
    * read out the display frame
    */
   clStatus = clEnqueueReadImage( cl_Commands, outputImageMemObj, CL_TRUE, origin, region, 0, 0, dispData, 0, nullptr, nullptr );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to read back final image data from warp kernel: " << clStatus;
       return false;
   }

   /*
    * read out the video frame
    */
   clStatus = clEnqueueReadImage( cl_Commands, outputVideoImageMemObj, CL_TRUE, origin, region, 0, 0, videoData, 0, nullptr, nullptr );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to read back video image data from warp kernel: " << clStatus;
       return false;
   }

   return true;
}


bool DSPGPU::loadFftOutMemoryObjects()
{
    cl_int err;

    fftRealOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
    fftImaginaryOutputMemObjSize = linesPerFrame * RescalingDataLength * sizeof(float);
     auto smi = SignalManager::instance();
    cl_bool isBlocking(CL_TRUE);

    if(SignalManager::instance()->isFftSource()){
        err = clEnqueueWriteBuffer (
                    cl_Commands,
                    fftRealOutputMemObj,
                    isBlocking,
                    0,
                    fftRealOutputMemObjSize,
                    smi->getRealDataPointer(),
                    0,
                    nullptr,
                    nullptr);

        if( err != CL_SUCCESS )
        {
             displayFailureMessage( tr( "Failed to init fftRealOutputMemObj" ), true );
             return false;
        }

        err = clEnqueueWriteBuffer (
                    cl_Commands,
                    fftImaginaryOutputMemObj,
                    isBlocking,
                    0,
                    fftImaginaryOutputMemObjSize,
                    smi->getImagDataPointer(),
                    0,
                    nullptr,
                    nullptr);

        if( err != CL_SUCCESS )
        {
             displayFailureMessage( tr( "Failed to init fftImaginaryOutputMemObj" ), true );
             return false;
        }
    }
    return true;
}

void DSPGPU::setAveraging(bool enable)
{
    doAveraging = enable;
}

void DSPGPU::setFrameAverageWeights(int inPrevFrameWeight_percent, int inCurrFrameWeight_percent)
{
    prevFrameWeight_percent = inPrevFrameWeight_percent / 100.0f;
    currFrameWeight_percent = inCurrFrameWeight_percent / 100.0f;
}

void DSPGPU::setDisplayAngle(float angle)
{
    displayAngle_deg = angle;
}
