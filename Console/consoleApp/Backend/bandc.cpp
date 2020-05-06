#include "bandc.h"
#include <util.h>

#include <QDebug>

BeAndCe::BeAndCe(cl_context context) : KernelFunctionBase(context),
    m_beAndCeGlobalWorkSize{
        size_t(FFTDataSize),
        size_t(m_signalModel->linesPerRevolution())
        }
{
    initContext();
}

BeAndCe::~BeAndCe()
{
    clReleaseMemObject(m_beAndCeImage );
}

bool BeAndCe::enqueueCallKernelFunction()
{
    if(m_kernel){
        setKernelArguments(m_kernel);
    }

    cl_int clStatus = clEnqueueNDRangeKernel( m_openClCommandQueue,
                                              m_kernel,
                                              m_signalModel->m_oclWorkDimension,
                                              m_signalModel->m_oclGlobalWorkOffset,
                                              m_beAndCeGlobalWorkSize,
                                              m_signalModel->m_oclLocalWorkSize,
                                              m_signalModel->m_numEventsInWaitlist,
                                              nullptr,
                                              nullptr );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to enqueue B and C kernel, reason: " << clStatus;
        return false;
    }
    return clStatus == CL_SUCCESS;
}

bool BeAndCe::initContext()
{
    bool success{false};
    success = createBeAndCeImageBuffer(m_baseContext);
    if(success) {
        m_signalModel->setPostBandcImageBuffer(m_beAndCeImage);
    }
    return success;
}

void BeAndCe::setKernel(cl_kernel kernel)
{
    if(!m_kernel){
        m_kernel = kernel;
        setKernelArguments(kernel);
    }
}

bool BeAndCe::setKernelArguments(cl_kernel kernel)
{
    if(!kernel || !m_signalModel){
        return false;
    }

    const auto* smi = SignalModel::instance();
    //lcv debug Brightness and contrast
    float br = 227.0f + *(smi->whiteLevel());
    float cntr = 27.0f+ *(smi->blackLevel());


    cl_int clStatus  = clSetKernelArg( kernel, 0, sizeof(cl_mem), smi->fftImageBuffer() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 0 , err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 1, sizeof(cl_mem), &m_beAndCeImage );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 1, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 2, sizeof(float), &cntr); //smi->blackLevel() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 2, err: "  << clStatus;
    }
    clStatus |= clSetKernelArg( kernel, 3, sizeof(float), &br); //smi->whiteLevel() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to set B and C argument 3, err: "  << clStatus;
    }

    return true;
}

bool BeAndCe::createBeAndCeImageBuffer(cl_context context)
{
    cl_int err{-1};

    const cl_image_format* imageFormat =  m_imageDescriptor.getImageFormat();

    const cl_image_desc* inputImageDescriptor = m_imageDescriptor.getBeAndCeImageDescriptor();

    m_beAndCeImage   = clCreateImage ( context, CL_MEM_READ_WRITE, imageFormat, inputImageDescriptor, nullptr, &err );
    if( err != CL_SUCCESS )
    {
        displayFailureMessage( QString( "Failed to create GPU images" ), true );
    }

    return err == CL_SUCCESS;
}
