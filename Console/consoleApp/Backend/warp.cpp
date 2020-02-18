#include "warp.h"
#include <util.h>

#include <QDebug>

Warp::Warp(cl_context context) :
    KernelFunctionBase(context),
    m_warpGlobalWorkSize{
        size_t(SectorWidth_px),
        size_t(SectorHeight_px)
        }
{
    initContext();
}

Warp::~Warp()
{
    clReleaseMemObject( m_warpImageBuffer );
    clReleaseMemObject( m_warpVideoBuffer );
}

bool Warp::enqueueCallKernelFunction()
{
    if(!m_signalModel){
        return false;
    }

    cl_int clStatus = clEnqueueNDRangeKernel( m_openClCommandQueue,
                                              m_kernel,
                                              m_signalModel->m_oclWorkDimension,
                                              m_signalModel->m_oclGlobalWorkOffset,
                                              m_warpGlobalWorkSize,
                                              m_signalModel->m_oclLocalWorkSize,
                                              m_signalModel->m_numEventsInWaitlist,
                                              nullptr,
                                              nullptr );

    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to execute warp kernel:" << clStatus;
        return false;
    }

    return clStatus == CL_SUCCESS;
}

bool Warp::initContext()
{
    bool success{false};
    success = createWarpImageBuffers(m_baseContext);
    return success;
}

void Warp::setKernel(cl_kernel kernel)
{
    if(!m_kernel){
        m_kernel = kernel;
        setKernelArguments(kernel);
    }
}

bool Warp::setKernelArguments(cl_kernel kernel)
{
    auto smi = SignalModel::instance();

    cl_int clStatus  = clSetKernelArg( kernel,  0, sizeof(cl_mem), smi->getBeAndCeImageBuffer() );

    clStatus |= clSetKernelArg( kernel,  1, sizeof(cl_mem), &m_warpImageBuffer );
    clStatus |= clSetKernelArg( kernel,  2, sizeof(cl_mem), &m_warpVideoBuffer );
    clStatus |= clSetKernelArg( kernel,  3, sizeof(cl_float),  smi->getCatheterRadiusUm() );
    clStatus |= clSetKernelArg( kernel,  4, sizeof(cl_float),  smi->getInternalImagingMaskPx() );
    clStatus |= clSetKernelArg( kernel,  5, sizeof(cl_float),  smi->getStandardDehthMm() );
    clStatus |= clSetKernelArg( kernel,  6, sizeof(cl_int),    smi->getStandardDehthS() );
    clStatus |= clSetKernelArg( kernel,  7, sizeof(cl_float),  smi->getDisplayAngle() );
    clStatus |= clSetKernelArg( kernel,  8, sizeof(cl_int),    smi->getReverseDirection() );
    clStatus |= clSetKernelArg( kernel,  9, sizeof(int),    &SectorWidth_px );
    clStatus |= clSetKernelArg( kernel, 10, sizeof(int),    &SectorHeight_px );
    clStatus |= clSetKernelArg( kernel, 11, sizeof(cl_float), smi->getFractionOfCanvas()  );
    clStatus |= clSetKernelArg( kernel, 12, sizeof(cl_int),  smi->getImagingDepth() );

    return  clStatus == CL_SUCCESS;
}

bool Warp::createWarpImageBuffers(cl_context context)
{
    cl_int err{-1};

    const cl_image_format* warpImageFormat = m_imageDescriptor.getImageFormat();

    const cl_image_desc* warpImageDescriptor = m_imageDescriptor.getWarpImageDescriptor();

    m_warpImageBuffer      = clCreateImage( context, CL_MEM_WRITE_ONLY, warpImageFormat, warpImageDescriptor, nullptr, &err );
    if( err != CL_SUCCESS )
    {
        displayFailureMessage( QString( "Failed to create GPU warp image buffer" ), true );
        return false;
    }
    m_signalModel->setWarpImageBuffer(m_warpImageBuffer);

    m_warpVideoBuffer = clCreateImage( context, CL_MEM_WRITE_ONLY, warpImageFormat, warpImageDescriptor, nullptr, &err );
    if( err != CL_SUCCESS )
    {
        displayFailureMessage( QString( "Failed to create GPU warp video buffer" ), true );
        return false;
    }
    m_signalModel->setWarpVideoBuffer(m_warpVideoBuffer);

    return true;
}
