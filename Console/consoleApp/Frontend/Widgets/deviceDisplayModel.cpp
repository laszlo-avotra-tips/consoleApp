#include "deviceDisplayModel.h"

DeviceDisplayModel::DeviceDisplayModel(const QString &name, const QImage& image)
    : m_name(name), m_image(image)
{

}

QString DeviceDisplayModel::name() const
{
    return m_name;
}

QImage DeviceDisplayModel::image() const
{
    return m_image;
}
