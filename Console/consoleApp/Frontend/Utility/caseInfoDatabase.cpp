#include "caseInfoDatabase.h"
#include "logger.h"

CaseInfoDatabase::CaseInfoDatabase()
{

}

QSqlError CaseInfoDatabase::initDb()
{
    QSqlError sqlerr;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName( m_dbName );

    if( !db.open() )
    {
        sqlerr =  db.lastError();
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }

    return sqlerr;
}
