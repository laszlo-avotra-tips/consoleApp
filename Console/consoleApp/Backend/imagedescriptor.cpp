#include "imagedescriptor.h"
#include <defaults.h>
#include <signalmodel.h>
#include "deviceSettings.h"

ImageDescriptor::ImageDescriptor() :
    m_imageWidth{FFT_DATA_SIZE}, m_imageHeight{size_t(MAX_LINES_PER_FRAME)},
    m_warpImageWidth{SECTOR_HEIGHT_PX}, m_warpImageHeight{SECTOR_HEIGHT_PX},
//    m_imageWidth{MaxALineLength}, m_imageHeight{size_t( SignalModel::instance()->linesPerRevolution())},
//    m_warpImageWidth{SectorWidth_px}, m_warpImageHeight{SectorHeight_px},
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
    auto& setting = deviceSettings::Instance();

    if(setting.getIsDeviceSimulation()){
        m_imageWidth = MaxALineLength;
        m_imageHeight = SignalModel::instance()->linesPerRevolution();
        m_warpImageWidth = SectorWidth_px;
        m_warpImageHeight = SectorHeight_px;

        m_inputImageDescriptor = {  //inputImageDescriptor
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
            };

        warpImageDescriptor = {
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
        };
    }
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
