#include "kernelfunctionlogarithmicpowerdensity.h"
#include <deviceSettings.h>

#include <QDebug>

KernelFunctionLogarithmicPowerDensity::KernelFunctionLogarithmicPowerDensity(cl_context context) : m_context(context)
{
    //create image
    if(m_context){
        m_image = createImageBuffer();
        if(!m_image){
            qDebug() << "failed to create image";
        }
    }
}

KernelFunctionLogarithmicPowerDensity::~KernelFunctionLogarithmicPowerDensity()
{
    //release image
    clReleaseMemObject( m_image );
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

void KernelFunctionLogarithmicPowerDensity::setFftBuffers(cl_mem real, cl_mem imag)
{
    m_fftRealBuffer = real;
    m_fftImagBuffer = imag;
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

    const size_t image_height{size_t(deviceSettings::Instance().current()->getLinesPerRevolution())}; //input_image_height

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
//        displayFailureMessage( tr( "Failed to create GPU images" ), true );
        return nullptr;
    }

    return m_image;
}
