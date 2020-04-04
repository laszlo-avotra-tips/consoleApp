#include "imagedescriptor.h"
#include <defaults.h>
#include <signalmodel.h>

ImageDescriptor::ImageDescriptor() :
    m_imageWidth{MaxALineLength}, m_imageHeight{size_t( SignalModel::instance()->linesPerRevolution())},
    m_warpImageWidth{SectorWidth_px}, m_warpImageHeight{SectorHeight_px},
    m_inputImageDescriptor{  //inputImageDescriptor
            m_imageType,
            m_imageWidth,
            m_imageHeight,
            m_imageDepth,
            m_imageArraySize,
            m_imageRowPitch,
            m_imageSlicePitch,
            m_numMipLevels,
            m_numSamples,
            {m_buffer}
        },
    warpImageDescriptor{
        m_imageType,
        m_warpImageWidth,
        m_warpImageHeight,
        m_imageDepth,
        m_imageArraySize,
        m_imageRowPitch,
        m_imageSlicePitch,
        m_numMipLevels,
        m_numSamples,
        {m_buffer}
    }
{

}

const cl_image_desc* ImageDescriptor::getFftImageDescriptor() const
{
    return getInputImageDescriptor();
}

const cl_image_desc* ImageDescriptor::getBeAndCeImageDescriptor() const
{
    return getInputImageDescriptor();
}

const cl_image_desc* ImageDescriptor::getWarpImageDescriptor() const
{
    return &warpImageDescriptor;
}

const cl_image_desc *ImageDescriptor::getOutputVideoImageDescriptor() const
{
    return &warpImageDescriptor;
}

const cl_image_desc *ImageDescriptor::getOutputImageDescriptor() const
{
    return &warpImageDescriptor;
}

const cl_image_format* ImageDescriptor::getImageFormat() const
{
    return &m_imageFormat;
}

const cl_image_desc* ImageDescriptor::getInputImageDescriptor() const
{
    return &m_inputImageDescriptor;
}
