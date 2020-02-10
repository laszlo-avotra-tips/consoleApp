#include "kernelfunctionlogarithmicpowerdensity.h"
#include <deviceSettings.h>
#include "CL/opencl.h"
#include <signalmanager.h>

#include <QDebug>

KernelFunctionLogarithmicPowerDensity::KernelFunctionLogarithmicPowerDensity() :
     m_linesPerRevolution(cl_uint(deviceSettings::Instance().current()->getLinesPerRevolution()))
{
}

KernelFunctionLogarithmicPowerDensity::KernelFunctionLogarithmicPowerDensity(cl_context context)
    : m_context(context), m_linesPerRevolution(cl_uint(deviceSettings::Instance().current()->getLinesPerRevolution()))
{
    init();
}

bool KernelFunctionLogarithmicPowerDensity::init()
{
    if(m_context){
        //create image
        m_image = createImageBuffer();
        if(!m_image){
            qDebug() << "failed to create image";
        }
        createFftBuffers();
        setKernelParameters();
        return true;
    }
    return false;
}

KernelFunctionLogarithmicPowerDensity::~KernelFunctionLogarithmicPowerDensity()
{
    //release image
    clReleaseMemObject(m_image );
    clReleaseMemObject(m_fftRealBuffer);
    clReleaseMemObject(m_fftImagBuffer);
}

bool KernelFunctionLogarithmicPowerDensity::enqueueInputGpuMemory(cl_command_queue cmds)
{
    cl_bool isBlocking(CL_TRUE);
    const size_t memSize {m_inputLength * m_linesPerRevolution * sizeof(float)};

    cl_int err = clEnqueueWriteBuffer (
                cmds,
                m_fftRealBuffer,
                isBlocking,
                0,
                memSize,
                SignalManager::instance()->getRealDataPointer(),
                0,
                nullptr,
                nullptr);

    if(err == CL_SUCCESS){
        err = clEnqueueWriteBuffer (
                    cmds,
                    m_fftImagBuffer,
                    isBlocking,
                    0,
                    memSize,
                    SignalManager::instance()->getImagDataPointer(),
                    0,
                    nullptr,
                    nullptr);
    }

    return err == CL_SUCCESS;
}

bool KernelFunctionLogarithmicPowerDensity::enqueueCallKernelFunction(cl_command_queue cmds)
{
    const size_t globalWorkSize[] {size_t(FFTDataSize),size_t(m_linesPerRevolution)};

    cl_int clStatus = clEnqueueNDRangeKernel( cmds, m_kernel,
                                              m_oclWorkDimension, m_oclGlobalWorkOffset, globalWorkSize,
                                              m_oclLocalWorkSize, m_numEventsInWaitlist, nullptr, nullptr );
    return clStatus != CL_SUCCESS;
}

bool KernelFunctionLogarithmicPowerDensity::enqueueOutputGpuMemory(cl_command_queue /*cmds*/)
{
    return false;
}

void KernelFunctionLogarithmicPowerDensity::setContext(cl_context context)
{
    m_context = context;
}

void KernelFunctionLogarithmicPowerDensity::setIsAveraging(bool isAveraging){
    m_isAveraging = isAveraging;
}

void KernelFunctionLogarithmicPowerDensity::setIsInvertColors(bool isInvertColors)
{
    m_isInvertColors = isInvertColors;
}

void KernelFunctionLogarithmicPowerDensity::createFftBuffers()
{
    cl_int err{-1};
    const size_t memSize {m_inputLength * m_linesPerRevolution * sizeof(float)};
    m_fftRealBuffer = clCreateBuffer( m_context, CL_MEM_READ_WRITE, memSize, nullptr , &err );
    if( err != CL_SUCCESS )
    {
         displayFailureMessage( "Failed to create fftImaginaryOutputMemObj", true );
    }

    m_fftImagBuffer = clCreateBuffer( m_context, CL_MEM_READ_WRITE, memSize, nullptr , &err );
    if( err != CL_SUCCESS )
    {
         displayFailureMessage( "Failed to create fftImaginaryOutputMemObj", true );
    }
}

cl_mem KernelFunctionLogarithmicPowerDensity::getFftRealBuffer() const
{
    return m_fftRealBuffer;
}

cl_mem KernelFunctionLogarithmicPowerDensity::getFftImagBuffer() const
{
    return m_fftImagBuffer;
}

void KernelFunctionLogarithmicPowerDensity::createLastFrameBuffer()
{

}

void KernelFunctionLogarithmicPowerDensity::setLastFrameBuffer(cl_mem lastFrameBuffer)
{
    m_lastFrameBuffer = lastFrameBuffer;
}

cl_mem KernelFunctionLogarithmicPowerDensity::getImageBuffer() const
{
    return m_image;
}

cl_mem KernelFunctionLogarithmicPowerDensity::createImageBuffer()
{
    cl_int err{-1};

    const cl_image_format clImageFormat{CL_R,CL_UNSIGNED_INT8};

    const cl_mem_object_type image_type{CL_MEM_OBJECT_IMAGE2D};

    const size_t image_width{MaxALineLength}; //input_image_width

    const size_t image_height{size_t(m_linesPerRevolution)}; //input_image_height

    const size_t image_depth{1};
    const size_t image_array_size{1};
    const size_t image_row_pitch{0};
    const size_t image_slice_pitch{0};
    const cl_uint num_mip_levels{0};
    const cl_uint num_samples{0};
    cl_mem buffer{nullptr};

    const cl_image_desc imageDescriptor{  //inputImageDescriptor
        image_type,
        image_width,
        image_height,
        image_depth,
        image_array_size,
        image_row_pitch,
        image_slice_pitch,
        num_mip_levels,
        num_samples,
        {buffer}
    };

    m_image  = clCreateImage ( m_context, CL_MEM_READ_WRITE, &clImageFormat, &imageDescriptor, nullptr, &err ); //inputImageMemObj
    if( err != CL_SUCCESS )
    {
        displayFailureMessage("Failed to create GPU images", true );
        return nullptr;
    }

    return m_image;
}

void KernelFunctionLogarithmicPowerDensity::setKernel(cl_kernel kernel)
{
    m_kernel = kernel;
    init();
}

void KernelFunctionLogarithmicPowerDensity::displayFailureMessage(const char *msg, bool isMajor) const
{
    qDebug() << msg << ", is major error = " <<isMajor;
}

bool KernelFunctionLogarithmicPowerDensity::setKernelParameters()
{
    if(!m_kernel){
        return false;
    }

    cl_int clStatus  = clSetKernelArg( m_kernel, 0, sizeof(cl_mem), &m_fftRealBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 0 , err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 1, sizeof(cl_mem), &m_fftImagBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 1, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 2, sizeof(cl_mem), &m_lastFrameBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 2, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 3, sizeof(cl_mem), &m_image );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 3, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 4, sizeof(int), &m_inputLength );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 4, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 5, sizeof(float), &m_scaleFactor );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 5, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 6, sizeof(unsigned int), &m_dcNoiseLevel );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 6, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 7, sizeof(int), &m_isAveraging );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 7, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 8, sizeof(float), &m_prevFrameWeight_percent );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 8, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 9, sizeof(float), &m_currFrameWeight_percent );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 9, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( m_kernel, 10, sizeof(int), &m_isInvertColors );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 10, err: "  << clStatus;
    }


    return true;
}
