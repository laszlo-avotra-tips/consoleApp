#ifndef LOCATIONMODEL_H
#define LOCATIONMODEL_H

#include <QStringList>
#include <QString>

class LocationModel
{
public:
    static LocationModel* instance();
    void addLocation(const QString& ln);

    QStringList locations() const;
    void setLocations(const QStringList &physicianNames);

    QString selectedLocation() const;
    void setSelectedLocation(const QString &selectedPysicianName);

private:
    LocationModel() = default;
    static LocationModel* m_instance;

    QStringList m_locations{{"Lab 1"}, {"Lab 2"}, {"Lab 3"}};
    QString m_selectedLocation{"Lab 2"};
};

#endif // LOCATIONMODEL_H
