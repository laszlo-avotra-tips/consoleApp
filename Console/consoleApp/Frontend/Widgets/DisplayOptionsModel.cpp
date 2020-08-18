#include "DisplayOptionsModel.h"

DisplayOptionsModel* DisplayOptionsModel::m_instance{nullptr};

DisplayOptionsModel *DisplayOptionsModel::instance()
{
    if(!m_instance){
        m_instance = new DisplayOptionsModel();
    }
    return m_instance;
}

DisplayOptionsModel::DisplayOptionsModel()
{

}

uint8_t DisplayOptionsModel::depthIndex() const
{
    return m_depthIndex;
}

void DisplayOptionsModel::setDepthIndex(const uint8_t &depthIndex)
{
    m_depthIndex = depthIndex;
}

int DisplayOptionsModel::imageContrast() const
{
    return m_imageContrast;
}

void DisplayOptionsModel::setImageContrast(int imageContrastIndex)
{
    m_imageContrast = imageContrastIndex;
}

int DisplayOptionsModel::imageBrightness() const
{
    return m_imageBrightness;
}

void DisplayOptionsModel::setImageBrightness(int brightness)
{
    m_imageBrightness = brightness;
}

int DisplayOptionsModel::reticleBrightness() const
{
    return m_reticleBrightness;
}

void DisplayOptionsModel::setReticleBrightness(const int &reticleBrightmness)
{
    m_reticleBrightness = reticleBrightmness;
}

bool DisplayOptionsModel::isImageColorGray() const
{
    return m_isImageColorGray;
}

void DisplayOptionsModel::setIsImageColorGray(bool isImageColorGray)
{
    m_isImageColorGray = isImageColorGray;
}

bool DisplayOptionsModel::isPointedDown() const
{
    return m_isPointedDown;
}

void DisplayOptionsModel::setIsPointedDown(bool isPointedDown)
{
    m_isPointedDown = isPointedDown;
}
