#ifndef CASEINFORMATIONMODEL_H
#define CASEINFORMATIONMODEL_H

#include <QStringList>
#include <QString>

class CaseInformationModel
{
public:
    CaseInformationModel();

    QStringList physicianNames() const;

    QString selectedPhysicianName() const;
    void setSelectedPhysicianName(const QString &selectedPhysicianName);
    bool isSelectedPhysicianName() const;
    void addPhysicianName(const QString& name);
    void setPhysicianName(int index, const QString &name);

    QStringList locations() const;

    QString selectedLocation() const;
    void setSelectedLocation(const QString &selectedLocation);
    bool isSelectedLocation() const;
    void addLocation(const QString& location);
    void setLocation(int index, const QString &location);


    QString patientId() const;
    void setPatientId(const QString &patientId);

    void resetModel();

    QString dateAndTime() const;
    void setDateAndTime(const QString &dateAndTime);

private:
    QStringList m_physicianNames{{"DR. Himanshu Patel"}, {"DR. Jaafer Golzar"}, {"DR. Kara Parker-Smith"}};
    QString m_selectedPhysicianName;
    QString m_defaultPhysicianName;

    QStringList m_locations{{"CATH LAB 1"}, {"CATH LAB 2"}, {"CATH LAB 3"}};
    QString m_selectedLocation;

    QString m_patientId;
    QString m_dateAndTime;
};

#endif // CASEINFORMATIONMODEL_H
