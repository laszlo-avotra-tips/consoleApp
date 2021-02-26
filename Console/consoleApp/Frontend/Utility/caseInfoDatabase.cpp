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
        return sqlerr;
    }
    // Setup the database only in the case
    // that the tables don't already exist
    QStringList tables = db.tables();

    // Set up the schema version table
    if( !tables.contains( "version", Qt::CaseInsensitive ) )
    {
        // Doesn't exist, create it.
        QSqlQuery q;
        if( !q.exec(QLatin1String("create table version(id integer primary key, tablename varchar, version integer)") ) )
        {
            sqlerr = q.lastError();
            const QString& errorMsg = sqlerr.databaseText();
            LOG1(errorMsg)
            return sqlerr;
        }
    }

    // Set up the Captures table
    if( !tables.contains( "captures", Qt::CaseInsensitive ) )
    {

        // Doesn't exist, create it.
        QSqlQuery q;
        if( !q.exec(QLatin1String("create table captures(id integer primary key, timestamp timestamp, tag varchar, name varchar, deviceName varchar, pixelsPerMm int)") ) )
        {
            sqlerr = q.lastError();
            const QString& errorMsg = sqlerr.databaseText();
            LOG1(errorMsg)
            return sqlerr;
        }
    }

    LOG1(sqlerr.databaseText())

    return sqlerr;
}
