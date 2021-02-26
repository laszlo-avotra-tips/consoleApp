#ifndef CASEINFODATABASE_H
#define CASEINFODATABASE_H

#include <QtSql>

class CaseInfoDatabase
{
public:
    CaseInfoDatabase();
    QSqlError initDb(void);

private:
    const QString m_dbName{"C:/Avinger_Data/caseInfo.db"};
};

#endif // CASEINFODATABASE_H

