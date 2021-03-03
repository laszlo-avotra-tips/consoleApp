#include "caseInfoDatabase.h"
#include "logger.h"

CaseInfoDatabase::CaseInfoDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName( m_dbName );

    if( !m_db.open() )
    {
        QSqlError sqlerr =  m_db.lastError();
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }
    createTables();
}

CaseInfoDatabase::~CaseInfoDatabase()
{
    if(m_db.open()){
        m_db.close();
    }
}

QSqlError CaseInfoDatabase::createTables()
{
    QSqlError sqlerr;

    // Setup the database only in the case
    // that the tables don't already exist
    QStringList tables = m_db.tables();

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
    return sqlerr;
}

int CaseInfoDatabase::addPhysician(const QString &name)
{
    QSqlQuery q;
    QSqlError sqlerr;

    q.prepare( QString("INSERT INTO Physicians (name)"
               "VALUES (?)") );
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

    int numLocations{0};
    while(q.next()){
        ++numLocations;
        record = q.record();
        const auto& location = (record.value(0).toString());
        cim->addLocation(location);
    }

    q.prepare( "SELECT name FROM Physicians ORDER BY name ASC" );

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }
    record = q.record();

    int numPhysicians{0};
    while(q.next()){
        ++numPhysicians;
        record = q.record();
        const auto& physician = (record.value(0).toString());
        cim->addPhysicianName(physician);
    }
    cim->initDefaults();
}

void CaseInfoDatabase::updatePhysicianTable(const std::set<QString> &names)
{
    QSqlError sqlerr;
    QSqlQuery q;
    q.prepare( QString("DELETE FROM Physicians"));

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }
    for(const auto& name : names){
        q.prepare( QString("INSERT INTO Physicians (name)"
                   "VALUES (?)") );
        q.addBindValue( name );

        q.exec();
        sqlerr = q.lastError();
        if(sqlerr.isValid()){
            const QString& errorMsg = sqlerr.databaseText();
            LOG1(errorMsg)
         }
    }
}

void CaseInfoDatabase::updateLocationTable(const std::set<QString> &names)
{
    QSqlError sqlerr;
    QSqlQuery q;
    q.prepare( QString("DELETE FROM Locations"));

    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }
    for(const auto& name : names){
        //ciDb.addPhysician(physician);
        q.prepare( QString("INSERT INTO Locations (name)"
                   "VALUES (?)") );
        q.addBindValue( name );

        q.exec();
        sqlerr = q.lastError();
        if(sqlerr.isValid()){
            const QString& errorMsg = sqlerr.databaseText();
            LOG1(errorMsg)
         }
    }
}
