#ifndef PREFERENCESMODEL_H
#define PREFERENCESMODEL_H

#include <QObject>
#include <set>

class PreferencesDatabase;

using PreferenceContainers_t = std::set<QString>;

class PreferencesModel
{
public:
    static PreferencesModel* instance();

    const PreferenceContainers_t& physicians() const;
    void setPhysicians(const PreferenceContainers_t &physicians);
    void addPhysician(const QString &physician);
    bool removePhysician(const QString &physician);

    const PreferenceContainers_t& locations() const;
    void setLocations(const PreferenceContainers_t &locations);
    void addLocation(const QString &location);
    bool removeLocation(const QString &location);

    QString defaultPhysician() const;
    void setDefaultPhysician(const QString &defaultPhysician);

    QString defaultLocation() const;
    void setDefaultLocation(const QString &defaultLocation);
    void persistPreferences();
    void loadPreferences();

    QString getSerialNumber() const;

private:
    PreferencesModel();
    static PreferencesModel* m_instance;

    void addPreference(PreferenceContainers_t& container, const QString& name);
    bool removePreference(PreferenceContainers_t& container, const QString& name);

    PreferenceContainers_t m_physicians;
    PreferenceContainers_t m_locations;

    QString m_defaultPhysician;
    QString m_defaultLocation;
};

#endif // PREFERENCESMODEL_H
