#include "locationModel.h"

LocationModel* LocationModel::m_instance{nullptr};

LocationModel *LocationModel::instance()
{
    if(!m_instance){
        m_instance = new LocationModel();
    }
    return m_instance;
}

QStringList LocationModel::locations() const
{
    return m_locations;
}

void LocationModel::setLocations(const QStringList &locations)
{
    m_locations = locations;
}

QString LocationModel::selectedLocation() const
{
    return m_selectedLocation;
}

void LocationModel::setSelectedLocation(const QString &selectedLocation)
{
    m_selectedLocation = selectedLocation;
}
