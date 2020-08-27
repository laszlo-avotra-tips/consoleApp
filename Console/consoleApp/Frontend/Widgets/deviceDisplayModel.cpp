#include "deviceDisplayModel.h"
#include "logger.h"

DeviceDisplayModel::DeviceDisplayModel(const QString &name, const QImage& image)
    : m_name(name), m_image(image)
{
    LOG2(m_name, &m_image)
}

QString DeviceDisplayModel::name() const
{
    return m_name;
}

QImage DeviceDisplayModel::image() const
{
    return m_image;
}
