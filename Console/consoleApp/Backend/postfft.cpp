#include "postfft.h"
#include <deviceSettings.h>
#include "CL/opencl.h"
#include <signalmanager.h>

#include <QDebug>


PostFft::PostFft(cl_context context) :
    KernelFunctionBase(context),
    m_fftMemSize(
        *(m_signalModel->getIputLength()) * m_signalModel->linesPerRevolution() * sizeof(float)
        ),
    m_fftGlobalWorkSize{
        size_t(FFTDataSize),
        size_t(m_signalModel->linesPerRevolution())
        }
{
    initContext();
}

bool PostFft::initContext()
{
    bool success{false};
    success = createImageBuffer();
    if(success) {
        m_signalModel->setFftImageBuffer(m_fftImageBuffer);
        success = createFftBuffers();
    }
    if(success){
        success = createLastFrameBuffer();
    }
    return success;
}

PostFft::~PostFft()
{
    clReleaseMemObject(m_fftRealBuffer);
    clReleaseMemObject(m_fftImagBuffer);
    clReleaseMemObject(m_lastFrameBuffer);
    clReleaseMemObject(m_fftImageBuffer );
}

bool PostFft::enqueueInputBuffers(const float *imag, const float *real)
{
    cl_bool isBlocking(CL_TRUE);

    cl_int err = clEnqueueWriteBuffer (
                m_openClCommandQueue,
                m_fftRealBuffer,
                isBlocking,
                0,
                m_fftMemSize,
                real,
                0,
                nullptr,
                nullptr);

    if(err == CL_SUCCESS){
        err = clEnqueueWriteBuffer (
                    m_openClCommandQueue,
                    m_fftImagBuffer,
                    isBlocking,
                    0,
                    m_fftMemSize,
                    imag,
                    0,
                    nullptr,
                    nullptr);
    }

    return err == CL_SUCCESS;
}

bool PostFft::enqueueCallKernelFunction()
{
    if(m_kernel){
        setKernelArguments(m_kernel);
    }
    cl_int clStatus = clEnqueueNDRangeKernel( m_openClCommandQueue, m_kernel,
                                              m_signalModel->m_oclWorkDimension,
                                              m_signalModel->m_oclGlobalWorkOffset,
                                              m_fftGlobalWorkSize,
                                              m_signalModel->m_oclLocalWorkSize,
                                              m_signalModel->m_numEventsInWaitlist, nullptr, nullptr );
    return clStatus == CL_SUCCESS;
}

bool PostFft::createFftBuffers()
{
    if(!m_signalModel){
        return false;
    }

    const size_t memSize {*m_signalModel->getIputLength() * m_signalModel->linesPerRevolution() * sizeof(float)};

    cl_int err{-1};
    m_fftRealBuffer = clCreateBuffer( m_baseContext, CL_MEM_READ_WRITE, memSize, nullptr , &err );
    if( err != CL_SUCCESS )
    {
         displayFailureMessage( "Failed to create m_fftRealBuffer", true );
    }

    if(err == CL_SUCCESS){
        m_fftImagBuffer = clCreateBuffer( m_baseContext, CL_MEM_READ_WRITE, memSize, nullptr , &err );
        if( err != CL_SUCCESS )
        {
             displayFailureMessage( "Failed to create m_fftImagBuffer", true );
        }
    }

    return err == CL_SUCCESS;
}

bool PostFft::createLastFrameBuffer()
{
    if(!m_signalModel){
        return false;
    }

    cl_int err{-1};

    m_lastFrameBuffer = clCreateBuffer( m_baseContext, CL_MEM_READ_WRITE, m_fftMemSize, nullptr, &err ); //lastFramePreScalingMemObj

    return (err == CL_SUCCESS);
}

cl_mem* PostFft::getImageBuffer()
{
    return &m_fftImageBuffer;
}

bool PostFft::createImageBuffer()
{   
    cl_int err{-1};

    const cl_image_format*imageFormat = m_imageDescriptor.getImageFormat();

    const cl_image_desc* imageDescriptor = m_imageDescriptor.getFftImageDescriptor();

    m_fftImageBuffer  = clCreateImage ( m_baseContext, CL_MEM_READ_WRITE, imageFormat, imageDescriptor, nullptr, &err ); //inputImageMemObj

    return err == CL_SUCCESS;
}

void PostFft::setKernel(cl_kernel kernel)
{
    if(!m_kernel){
        m_kernel = kernel;
        setKernelArguments(kernel);
    }
}

void PostFft::displayFailureMessage(const char *msg, bool isMajor) const
{
    qDebug() << msg << ", is major error = " <<isMajor;
}

bool PostFft::setKernelArguments(cl_kernel kernel)
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
    clStatus |= clSetKernelArg( kernel, 3, sizeof(cl_mem), &m_fftImageBuffer );
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 3, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 4, sizeof(cl_int), m_signalModel->getIputLength());
    if( clStatus != CL_SUCCESS )
    {
       qDebug() << "DSP: Failed to set post processing argument 4, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 5, sizeof(cl_float), m_signalModel->scaleFactor() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 5, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 6, sizeof(cl_uint), m_signalModel->dcNoiseLevel() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 6, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 7, sizeof(cl_int), m_signalModel->isAveragingNoiseReduction() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 7, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 8, sizeof(cl_float), m_signalModel->prevFrameWeight_percent() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 8, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 9, sizeof(cl_float), m_signalModel->currFrameWeight_percent() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 9, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 10, sizeof(cl_int), m_signalModel->isInvertOctColors() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set post processing argument 10, err: "  << clStatus;
    }

    return clStatus == CL_SUCCESS;
}

