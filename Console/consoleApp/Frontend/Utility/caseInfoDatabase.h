#ifndef CASEINFODATABASE_H
#define CASEINFODATABASE_H

#include <QtSql>
#include "caseInformationModel.h"

class CaseInfoDatabase
{
public:
    CaseInfoDatabase();
    QSqlError initDb(void);
    int addPhysician(const QString& name);
    int addLocation(const QString& name);

    const PhysicianNameContainer& physicians() const;
    const PhysicianNameContainer& locations() const;

private:
    void initCaseInfo();

    const QString m_dbName{"C:/Avinger_Data/caseInfo.db"};
    PhysicianNameContainer m_physicians;
    PhysicianNameContainer m_locations;
};

#endif // CASEINFODATABASE_H

