#ifndef PHYSICIANNAMEMODEL_H
#define PHYSICIANNAMEMODEL_H

#include <QStringList>
#include <QString>

class PhysicianNameModel
{
public:
    static PhysicianNameModel* instance();
    void addPhysicianName(const QString& pn);


    QStringList physicianNames() const;
    void setPhysicianNames(const QStringList &physicianNames);

    QString selectedPysicianName() const;
    void setSelectedPysicianName(const QString &selectedPysicianName);

    QString defaultPysicianName() const;
    void setDefaultPysicianName(const QString &defaultPysicianName);

private:
    PhysicianNameModel() = default;
    static PhysicianNameModel* m_instance;

    QStringList m_physicianNames{{"DR. Patel"}, {"DR. Smith"}, {"DR. Johnes"}};
    QString m_selectedPysicianName{"DR. Patel"};
    QString m_defaultPysicianName;

};

#endif // PHYSICIANNAMEMODEL_H
