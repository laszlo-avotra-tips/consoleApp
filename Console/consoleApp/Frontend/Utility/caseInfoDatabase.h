#ifndef CASEINFODATABASE_H
#define CASEINFODATABASE_H

#include <QtSql>
#include "caseInformationModel.h"

class CaseInfoDatabase
{
public:
    CaseInfoDatabase();
    ~CaseInfoDatabase();

    int addPhysician(const QString& name);
    int addLocation(const QString& name);

    const PhysicianNameContainer& physicians() const;
    const PhysicianNameContainer& locations() const;
    void initCaseInfo();

private:
    QSqlError createTables();

    const QString m_dbName{"C:/Avinger_Data/caseInfo.db"};
    QSqlDatabase m_db;

    PhysicianNameContainer m_physicians;
    PhysicianNameContainer m_locations;
};

#endif // CASEINFODATABASE_H

