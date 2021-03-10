#include "preferencesModel.h"
#include "preferencesDatabase.h"
#include "userSettings.h"
#include <QStorageInfo>

PreferencesModel* PreferencesModel::m_instance = nullptr;

PreferencesModel::PreferencesModel()
{
    loadPreferences();

    QStorageInfo sInfo("/AvingerData");
    int mb = sInfo.bytesAvailable() / (1024 * 1024);
    m_spaceRemaining = QString::number(mb);
}

PreferencesModel *PreferencesModel::instance()
{
    if(!m_instance){
        m_instance = new PreferencesModel();
    }
    return m_instance;
}

const PreferenceContainers_t &PreferencesModel::physicians() const
{
    return m_physicians;
}

void PreferencesModel::setPhysicians(const PreferenceContainers_t &physicians)
{
    m_physicians = physicians;
}

void PreferencesModel::addPhysician(const QString &physician)
{
    addPreference(m_physicians, physician);
}

bool PreferencesModel::removePhysician(const QString &physician)
{
    return removePreference(m_physicians, physician);
}

const PreferenceContainers_t &PreferencesModel::locations() const
{
    return m_locations;
}

void PreferencesModel::setLocations(const PreferenceContainers_t &locations)
{
    m_locations = locations;
}

void PreferencesModel::addLocation(const QString &location)
{
    addPreference(m_locations, location);
}

bool PreferencesModel::removeLocation(const QString &location)
{
    return removePreference(m_locations, location);
}

QString PreferencesModel::defaultPhysician() const
{
    return m_defaultPhysician;
}

void PreferencesModel::setDefaultPhysician(const QString &defaultPhysician)
{
    m_defaultPhysician = defaultPhysician;
}

QString PreferencesModel::defaultLocation() const
{
    return m_defaultLocation;
}

void PreferencesModel::setDefaultLocation(const QString &defaultLocation)
{
    m_defaultLocation = defaultLocation;
}

void PreferencesModel::addPreference(PreferenceContainers_t &container, const QString &name)
{
    container.insert(name);
}

bool PreferencesModel::removePreference(PreferenceContainers_t &container, const QString &name)
{
    bool success{false};

    const auto num = container.erase(name);
    if(num){
        success = true;
    }
    return success;
}

QString PreferencesModel::getSpaceRemaining() const
{
    return m_spaceRemaining;
}

void PreferencesModel::loadPreferences()
{
    PreferencesDatabase db;
    db.initContainers();

    m_physicians = db.physicians();
    m_locations = db.locations();

    auto defaultPhysicianCandidate = userSettings::Instance().getPhysician();
    const auto phIt = m_physicians.find(defaultPhysicianCandidate);
    if(phIt != m_physicians.end()){
        m_defaultPhysician = defaultPhysicianCandidate;
    }

    auto defaultLocationCandidate = userSettings::Instance().getLocation();
    const auto locIt = m_locations.find(defaultLocationCandidate);
    if(locIt != m_locations.end()){
        m_defaultLocation = defaultLocationCandidate;
    }
}

QString PreferencesModel::getSerialNumber() const
{
    return userSettings::Instance().getSerialNumber();
}

void PreferencesModel::persistPreferences()
{
    PreferencesDatabase db;
    db.updatePhysicianTable(m_physicians);
    db.updateLocationTable(m_locations);

    userSettings::Instance().setPhysician(m_defaultPhysician);
    userSettings::Instance().setLocation(m_defaultLocation);
}
