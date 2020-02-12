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
#include "theglobals.h"
#include "signalmanager.h"
#include "playbackmanager.h"
#include "signalmodel.h"
#include "postfft.h"
#include "signalprocessingfactory.h"


/*
 * OpenCL core organization parameters
 */
#define DEFAULT_GLOBAL_UNITS ( 2048 )
#define DEFAULT_LOCAL_UNITS  ( 16 )


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
    auto smi = SignalModel::instance();
    smi->setAverageVal(doAveraging);

    displayAngle_deg = 0.0;

    smi->setCurrFrameWeight_percent(DefaultCurrFrameWeight_Percent / 100.0f);

    smi->setPrevFrameWeight_percent(1.0f - DefaultCurrFrameWeight_Percent / 100.0f);
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

void DSPGPU::initOpenClFileMap()
{
    m_openClFileMap = {
        {"postfft_kernel",  ":/kernel/postfft"},
        {"bandc_kernel",  ":/kernel/bandc"},
        {"warp_kernel",  ":/kernel/warp"}
    };
}

bool DSPGPU::callPostFftKernel()
{
    bool success{false};
    if(cl_Commands && m_postFft){
        success = m_postFft->enqueueCallKernelFunction(cl_Commands);
    }
    return success;
}

bool DSPGPU::callBandcKernel()
{
    auto itbc = m_openClFunctionMap.find("bandc_kernel");
    if(itbc != m_openClFunctionMap.end() && m_postFft)
    {
         auto& oclbck = itbc->second.second;
         cl_int clStatus  = clSetKernelArg( oclbck, 0, sizeof(cl_mem), SignalModel::instance()->postFftImageBuffer() );
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

         const size_t globalWorkSize[] {size_t(FFTDataSize),size_t(linesPerFrame)};

         clStatus = clEnqueueNDRangeKernel( cl_Commands, oclbck, oclWorkDimension, oclGlobalWorkOffset, globalWorkSize, oclLocalWorkSize, numEventsInWaitlist, nullptr, nullptr );
         if( clStatus != CL_SUCCESS )
         {
             qDebug() << "DSP: Failed to execute B and C kernel, reason: " << clStatus;
             return false;
         }
    }
    return true;
}

bool DSPGPU::callWarpKernel() const
{
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
        cl_int clStatus  = clSetKernelArg( oclwk,  0, sizeof(cl_mem), &warpInputImageMemObj );
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

        const size_t globalWorkSize[] {size_t(SectorWidth_px),size_t(SectorHeight_px)};

        clStatus = clEnqueueNDRangeKernel( cl_Commands, oclwk, oclWorkDimension, oclGlobalWorkOffset, globalWorkSize, oclLocalWorkSize, numEventsInWaitlist, nullptr, nullptr );

        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to execute warp kernel:" << clStatus;
            return false;
        }

        // Do all the work that was queued up on the GPU
        clStatus = clFinish( cl_Commands );
        if( clStatus != CL_SUCCESS )
        {
            qDebug() << "DSP: Failed to issue and complete all queued commands: " << clStatus;
            return false;
        }
    }
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

    auto spf = SignalProcessingFactory::instance();

    initOpenClFileMap();

    cl_Context = spf->getContext();
    cl_ComputeDeviceId = spf->getComputeDeviceId();

    cl_Commands = spf->getCommandQueue();
    m_postFft = spf->getPostFft();

    for( const auto& sourceCode : m_openClFileMap){
        const auto& kernelFunctionName = sourceCode.first;
        bool success = spf->buildKernelFuncionCode(kernelFunctionName);
        if(!success){
            return false;
        }
    }

    m_openClFunctionMap = spf->getOpenClFunctionMap();

    auto it = m_openClFunctionMap.find("postfft_kernel");//lcv get it from the factory
    if(it != m_openClFunctionMap.end())
    {
        m_postFft->setKernel(it->second.second);
    }

    createCLMemObjects( cl_Context );

    qDebug() << "DSPGPU: OpenCL init complete.";

    return true;
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

    if(!callPostFftKernel()){
        return false;
    }

    /*
     * Adjust brightness and contrast
     */
    if(!callBandcKernel()){
        return false;
    }

    if(!callWarpKernel()){
        return false;
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
    if(cl_Commands && m_postFft){
        m_postFft->enqueueInputGpuMemory(cl_Commands);
        return true;
    }
    return false;
}

void DSPGPU::setAveraging(bool enable)
{
    doAveraging = enable;
    SignalModel::instance()->setAverageVal(doAveraging);

}

void DSPGPU::setFrameAverageWeights(int inPrevFrameWeight_percent, int inCurrFrameWeight_percent)
{
    SignalModel::instance()->setPrevFrameWeight_percent(inPrevFrameWeight_percent / 100.0f);
    SignalModel::instance()->setCurrFrameWeight_percent(inCurrFrameWeight_percent / 100.0f);
}

void DSPGPU::setDisplayAngle(float angle)
{
    displayAngle_deg = angle;
}
