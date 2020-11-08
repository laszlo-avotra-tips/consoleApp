#ifndef DISPLAYOPTIONSMODEL_H
#define DISPLAYOPTIONSMODEL_H

#include <QObject>
#include <cstdint>

class DisplayOptionsModel
{
public:
    DisplayOptionsModel();

    bool isPointedDown() const;
    void setIsPointedDown(bool isPointedDown);

    bool isImageColorGray() const;
    void setIsImageColorGray(bool isImageColorGray);

    int reticleBrightness() const;
    void setReticleBrightness(const int &reticleBrightmness);

    int imageBrightness() const;
    void setImageBrightness(int brightness);

    int imageContrast() const;
    void setImageContrast(int imageContrast);

    uint8_t depthIndex() const;
    void setDepthIndex(const uint8_t &depthIndex);

    void persistModel() const;

    QString fileName() const;
    void setFileName(const QString &fileName);

    int bufferCount() const;
    void setBufferCount(int bufferCount);

private:
    bool m_isPointedDown{true};
    bool m_isImageColorGray{true};
    int  m_reticleBrightness{200};
    int  m_imageBrightness{0};
    int  m_imageContrast{1};
    uint8_t m_depthIndex{1};
    QString m_fileName;
    int     m_bufferCount{0};
};

#endif // DISPLAYOPTIONSMODEL_H
