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

    const QStringList& physicians() const;
    const QStringList& locations() const;

private:
    void initCaseInfo();

    const QString m_dbName{"C:/Avinger_Data/caseInfo.db"};
    QStringList m_physicians;
    QStringList m_locations;
};

#endif // CASEINFODATABASE_H

