#ifndef DEVICEDISPLAYMODEL_H
#define DEVICEDISPLAYMODEL_H

#include <QString>
#include <QImage>

class DeviceDisplayModel
{
public:
    DeviceDisplayModel(const QString& name, const QImage& image);

    QString name() const;

    QImage image() const;

private:
    const QString m_name;
    const QImage m_image;
};

#endif // DEVICEDISPLAYMODEL_H
