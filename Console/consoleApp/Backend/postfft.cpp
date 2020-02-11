#include "postfft.h"
#include <deviceSettings.h>
#include "CL/opencl.h"
#include <signalmanager.h>

#include <QDebug>


PostFft::PostFft(cl_context context)
{
    initContext(context);
}

bool PostFft::initContext(cl_context context)
{
    bool success{false};
    success = createImageBuffer(context);
    if(success) {
        m_signalModel->setPostFftImageBuffer(m_image);
        success = createFftBuffers(context);
    }
    if(success){
        success = createLastFrameBuffer(context);
    }
    return success;
}

PostFft::~PostFft()
{
    clReleaseMemObject(m_fftRealBuffer);
    clReleaseMemObject(m_fftImagBuffer);
    clReleaseMemObject(m_lastFrameBuffer);
    clReleaseMemObject(m_image );
}

bool PostFft::enqueueInputGpuMemory(cl_command_queue cmds)
{
    if(!m_signalModel){
        return false;
    }
    cl_bool isBlocking(CL_TRUE);
    const size_t memSize {*(m_signalModel->iputLength()) * m_signalModel->linesPerRevolution() * sizeof(float)};

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

bool PostFft::enqueueCallKernelFunction(cl_command_queue cmds)
{
    const size_t globalWorkSize[] {size_t(FFTDataSize),size_t(m_signalModel->linesPerRevolution())};

    cl_int clStatus = clEnqueueNDRangeKernel( cmds, m_kernel,
                                              m_oclWorkDimension, m_oclGlobalWorkOffset, globalWorkSize,
                                              m_oclLocalWorkSize, m_numEventsInWaitlist, nullptr, nullptr );
    return clStatus == CL_SUCCESS;
}

bool PostFft::createFftBuffers(cl_context context)
{
    if(!m_signalModel){
        return false;
    }

    const size_t memSize {*m_signalModel->iputLength() * m_signalModel->linesPerRevolution() * sizeof(float)};

    cl_int err{-1};
    m_fftRealBuffer = clCreateBuffer( context, CL_MEM_READ_WRITE, memSize, nullptr , &err );
    if( err != CL_SUCCESS )
    {
         displayFailureMessage( "Failed to create m_fftRealBuffer", true );
    }

    if(err == CL_SUCCESS){
        m_fftImagBuffer = clCreateBuffer( context, CL_MEM_READ_WRITE, memSize, nullptr , &err );
        if( err != CL_SUCCESS )
        {
             displayFailureMessage( "Failed to create m_fftImagBuffer", true );
        }
    }

    return err == CL_SUCCESS;
}

bool PostFft::createLastFrameBuffer(cl_context context)
{
    if(!m_signalModel){
        return false;
    }

    const size_t memSize {*m_signalModel->iputLength() *  m_signalModel->linesPerRevolution() * sizeof(float)};

    cl_int err{-1};

    m_lastFrameBuffer = clCreateBuffer( context, CL_MEM_READ_WRITE, memSize, nullptr, &err ); //lastFramePreScalingMemObj

    return (err == CL_SUCCESS);
}

cl_mem* PostFft::getImageBuffer()
{
    return &m_image;
}

bool PostFft::createImageBuffer(cl_context context)
{   
    if(!m_signalModel){
        return false;
    }
    cl_int err{-1};

    const cl_image_format clImageFormat{CL_R,CL_UNSIGNED_INT8};

    const cl_mem_object_type image_type{CL_MEM_OBJECT_IMAGE2D};

    const size_t image_width{MaxALineLength}; //input_image_width

    const size_t image_height{size_t( m_signalModel->linesPerRevolution())}; //input_image_height

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

    m_image  = clCreateImage ( context, CL_MEM_READ_WRITE, &clImageFormat, &imageDescriptor, nullptr, &err ); //inputImageMemObj

    return err == CL_SUCCESS;
}

void PostFft::setKernel(cl_kernel kernel)
{
    if(!m_kernel){
        m_kernel = kernel;
        setKernelParameters(kernel);
    }
}

void PostFft::displayFailureMessage(const char *msg, bool isMajor) const
{
    qDebug() << msg << ", is major error = " <<isMajor;
}

bool PostFft::setKernelParameters(cl_kernel kernel)
{
    if(!kernel || !m_signalModel){
        return false;
    }

    cl_int clStatus  = clSetKernelArg( kernel, 0, sizeof(cl_mem), &m_fftRealBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 0 , err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 1, sizeof(cl_mem), &m_fftImagBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 1, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 2, sizeof(cl_mem), &m_lastFrameBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 2, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 3, sizeof(cl_mem), &m_image );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 3, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 4, sizeof(int), m_signalModel->iputLength());
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 4, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 5, sizeof(float), m_signalModel->scaleFactor() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 5, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 6, sizeof(unsigned int), m_signalModel->dcNoiseLevel() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 6, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 7, sizeof(int), m_signalModel->averageVal() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 7, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 8, sizeof(float), m_signalModel->prevFrameWeight_percent() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 8, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 9, sizeof(float), m_signalModel->currFrameWeight_percent() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 9, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 10, sizeof(int), m_signalModel->isInvertColors() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 10, err: "  << clStatus;
    }

    return clStatus == CL_SUCCESS;
}

void PostFft::setSignalModel(SignalModel &signalModel)
{
    m_signalModel = &signalModel;
}
