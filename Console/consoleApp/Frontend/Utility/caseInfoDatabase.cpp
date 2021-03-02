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

    // Set up the schema Pysician table
    if( !tables.contains( "Physicians", Qt::CaseSensitive ) )
    {
        // Doesn't exist, create it.
        QSqlQuery q;
        if( !q.exec(QLatin1String("create table Physicians(id integer primary key, name varchar)") ) )
        {
            sqlerr = q.lastError();
            const QString& errorMsg = sqlerr.databaseText();
            LOG1(errorMsg)
            return sqlerr;
        }
    }

    // Set up the Captures table
    if( !tables.contains( "Locations", Qt::CaseInsensitive ) )
    {

        // Doesn't exist, create it.
        QSqlQuery q;
        if( !q.exec(QLatin1String("create table Locations(id integer primary key, name varchar)") ) )
        {
            sqlerr = q.lastError();
            const QString& errorMsg = sqlerr.databaseText();
            LOG1(errorMsg)
            return sqlerr;
        }
    }

    LOG1(sqlerr.databaseText());

    initCaseInfo();

    return sqlerr;
}

int CaseInfoDatabase::addPhysician(const QString &name)
{
    QSqlQuery q;
    QSqlError sqlerr;

    q.prepare( QString("INSERT INTO Physicians (name)"
               "VALUES (?)") );
    LOG1(name)
    q.addBindValue( name );

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
        return -1;
    }

    return 0;
}

int CaseInfoDatabase::addLocation(const QString &name)
{
    QSqlQuery q;
    QSqlError sqlerr;

    q.prepare( QString("INSERT INTO Locations (name)"
               "VALUES (?)") );
    LOG1(name)
    q.addBindValue( name );

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
        return -1;
    }

    return 0;
}

const PhysicianNameContainer &CaseInfoDatabase::physicians() const
{
    return m_physicians;
}

const PhysicianNameContainer &CaseInfoDatabase::locations() const
{
    return m_locations;
}

void CaseInfoDatabase::initCaseInfo()
{
    QSqlQuery q;
    QSqlError sqlerr;
    PhysicianNameContainer nl;

    auto cim = CaseInformationModel::instance();

    q.prepare( "SELECT name FROM Locations ORDER BY name ASC" );

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }
    QSqlRecord record = q.record();
    LOG1(record.count());

    int numLocations{0};
    LOG1(record.value(0).toString());
    LOG1(record.indexOf("name"));
    while(q.next()){
        ++numLocations;
        record = q.record();
        const auto& location = (record.value(0).toString());
        LOG1(location);
        cim->addLocation(location);
    }
    LOG2(numLocations, cim->locations().size());

    q.prepare( "SELECT name FROM Physicians ORDER BY name ACS" );

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }
    record = q.record();
    LOG1(record.count());

    int numPhysicians{0};
    LOG1(record.value(0).toString());
    LOG1(record.indexOf("name"));
    while(q.next()){
        ++numPhysicians;
        record = q.record();
        const auto& physician = (record.value(0).toString());
        LOG1(physician);
        cim->addPhysicianName(physician);
    }
    LOG2(numLocations, cim->physicianNames().size());
    cim->initDefaults();
}
