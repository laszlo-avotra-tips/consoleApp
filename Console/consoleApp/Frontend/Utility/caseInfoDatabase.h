#ifndef CASEINFODATABASE_H
#define CASEINFODATABASE_H

#include <QtSql>

class CaseInfoDatabase
{
public:
    CaseInfoDatabase();
    QSqlError initDb(void);
    int addPhysician(const QString& name);
    int addLocation(const QString& name);

private:
    const QString m_dbName{"C:/Avinger_Data/caseInfo.db"};
};

#endif // CASEINFODATABASE_H

