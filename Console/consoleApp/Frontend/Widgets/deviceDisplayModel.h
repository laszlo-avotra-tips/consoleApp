#ifndef DEVICEDISPLAYMODEL_H
#define DEVICEDISPLAYMODEL_H

#include <QString>
#include <QImage>

class DeviceDisplayModel
{
public:
    DeviceDisplayModel() = default;
    ~DeviceDisplayModel() = default;
    DeviceDisplayModel(const DeviceDisplayModel &) = default;

    DeviceDisplayModel(const QString& name, const QImage* image);

    QString name() const;

    QImage image() const;

private:
    const QString m_name;
    const QImage m_image;
};

Q_DECLARE_METATYPE(DeviceDisplayModel);

#endif // DEVICEDISPLAYMODEL_H
