#include "DisplayOptionsModel.h"
#include "Utility/userSettings.h"
#include "logger.h"


DisplayOptionsModel::DisplayOptionsModel()
{
    userSettings &settings = userSettings::Instance();
    m_imageContrast = settings.contrast();
    m_imageBrightness = settings.brightness();
    m_reticleBrightness = settings.reticleBrightness();
    LOG3(m_imageBrightness,m_imageContrast, m_reticleBrightness)

    m_isPointedDown = settings.isDistalToProximalView();
    m_isImageColorGray = settings.getIsGray();
    m_depthIndex = settings.getImageDepthIndex();
}

uint8_t DisplayOptionsModel::depthIndex() const
{
    return m_depthIndex;
}

void DisplayOptionsModel::setDepthIndex(const uint8_t &depthIndex)
{
    m_depthIndex = depthIndex;
}

void DisplayOptionsModel::persistModel() const
{
    userSettings &settings = userSettings::Instance();
    settings.setContrast(imageContrast());
    settings.setBrightness(imageBrightness());
    settings.setReticleBrightness(reticleBrightness());

    if(m_isPointedDown){
        settings.setCatheterView( userSettings::DistalToProximal );
    } else {
        settings.setCatheterView( userSettings::ProximalToDistal );
    }
    settings.setIsGray(m_isImageColorGray);
    settings.setImageDepthIndex(m_depthIndex);
    LOG2(m_isImageColorGray, m_depthIndex);
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

void DisplayOptionsModel::setReticleBrightness(const int &reticleBrightness)
{
    m_reticleBrightness = reticleBrightness;
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
