/*
 * sessiondatabase.cpp
 *
 * Handles all database operations for image captures.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#include "sessiondatabase.h"
#include "userSettings.h"
#include "defaults.h"
#include "util.h"
#include <QMap>
#include <QDateTime>

// Schema layouts for each table are defined in SDS 0007
#define IMAGE_CAPTURES_SCHEMA_VERSION  5
#define OCT_LOOPS_SCHEMA_VERSION       4
#define SESSION_SCHEMA_VERSION         3

/*
 * constructor
 */
sessionDatabase::sessionDatabase()
{
    // Find the database in the patient directory if it
    // exists, or create a new one.
    if( !QSqlDatabase::drivers().contains( "QSQLITE" ) )
    {
        // TBD:  this should fail earlier.  don't allow app to start up?
        displayFailureMessage( QObject::tr( "Unable to load database:\nThis application needs the SQLITE driver" ), true );
    }
}

/*
 * initDb()
 *
 * Create a new database using the session identifiers
 * specified by the user. Sets up the SQLite file,
 * the schema and tables.
 */
QSqlError sessionDatabase::initDb(void)
{
    caseInfo &info = caseInfo::Instance();

    // Not ready yet
    if( !info.storageValid() )
    {
        return QSqlError();
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName( info.getStorageDir().append( "/" ).append( SessionDatabaseFileName ) );
    QSqlError sqlerr;

    if( !db.open() )
    {
        displayFailureMessage( QObject::tr( "Database error:\nFailed to open capture database for session." ), true );
        return db.lastError();
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
            displayFailureMessage( QObject::tr( "Database Failure:\nFailed to create db schema - version table" ), true );
            return sqlerr;
        }
    }

    // Set up the Captures table
    if( !tables.contains( "captures", Qt::CaseInsensitive ) )
    {

        // Doesn't exist, create it.
        QSqlQuery q;
        if( !q.exec(QLatin1String("create table captures(id integer primary key, timestamp timestamp, tag varchar, name varchar, deviceName varchar, isHighSpeed char, pixelsPerMm int)") ) )
        {
            sqlerr = q.lastError();
            displayFailureMessage( QObject::tr( "Database Failure:\nFailed to create db schema - captures table" ), true );
            return sqlerr;
        }
    }

    // Set up the OCT Loops table
    if( !tables.contains( "octLoops", Qt::CaseInsensitive ) )
    {

        // Doesn't exist, create it.
        QSqlQuery q;
        if( !q.exec( QLatin1String( "create table octLoops(id integer primary key, timestamp timestamp, tag varchar, name varchar, length_ms integer, catheterView varchar, deviceName varchar, isHighSpeed char)" ) ) )
        {
            sqlerr = q.lastError();
            displayFailureMessage( QObject::tr( "Database Failure:\nFailed to create db schema - octLoops table" ), true );
            return sqlerr;
        }
    }

    // Set up the Session table
    if( !tables.contains( "session", Qt::CaseInsensitive ) )
    {

        // Doesn't exist, create it
        QSqlQuery q;
        if( !q.exec(QLatin1String("create table session(caseid varchar primary key, timestamp timestamp, utcOffset integer, patient varchar, doctor varchar, location varchar, notes varchar, cleanExit char)" ) ) )
        {
            sqlerr = q.lastError();
            displayFailureMessage( QObject::tr( "Database Failure:\nFailed to create db schema - session table" ), true );
            return sqlerr;
        }
    }

    // Add the current version numbers for this schema
    populateVersionTable();

    return sqlerr;
}

/*
 * populateVersionTable()
 *
 * Populate the version table with the current versions for each table.
 */
void sessionDatabase::populateVersionTable( void )
{
    QMap<QString, int> versionMap;

    // map the tables to their current version numbers
    versionMap[ "captures" ] = IMAGE_CAPTURES_SCHEMA_VERSION;
    versionMap[ "octLoops" ] = OCT_LOOPS_SCHEMA_VERSION;
    versionMap[ "session" ]  = SESSION_SCHEMA_VERSION;

    QSqlQuery q;
    QSqlError sqlerr;

    // Walk the list and add each table to the version table
    QMapIterator<QString, int> i( versionMap );
    while( i.hasNext() )
    {
        i.next();

        q.prepare( QString( "INSERT INTO version (tablename, version)"
                            "VALUES (?, ?)" ) );
        q.addBindValue( i.key() );
        q.addBindValue( i.value() );

        q.exec();

        sqlerr = q.lastError();
        if( sqlerr.isValid() )
        {
            displayFailureMessage( QObject::tr( "Database failure:Failed to INSERT new version data." ), true );
        }
    }
}

/*
 * close()
 *
 * Public call to close.
 */
void sessionDatabase::close( void )
{
    db.close();
}

/*
 * createSession()
 *
 * After the database has been set up, insert the session
 * data into the table.
 */
void sessionDatabase::createSession( void )
{
    caseInfo &info = caseInfo::Instance();
    QSqlQuery q;
    QSqlError sqlerr;

    // Session start time is UTC
    QString timeStr = QDateTime::currentDateTime().toUTC().toString( "yyyy-MM-dd HH:mm:ss" );

    q.prepare( QString( "INSERT INTO session (caseid, timestamp, utcOffset, patient, doctor, location, notes, cleanExit)"
                        "VALUES (?, ?, ?, ?, ? ,?, ?, ?)" ) );
    q.addBindValue( info.getCaseID() );
    q.addBindValue( timeStr );
    q.addBindValue( info.getUtcOffset() );
    q.addBindValue( info.getPatientID() );
    q.addBindValue( info.getDoctor() );
    q.addBindValue( info.getLocation() );
    q.addBindValue( info.getNotes() );
    q.addBindValue( false );
    q.exec();

    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        displayFailureMessage( QObject::tr( "Database failure:\nFailed to INSERT new session data." ), true );
    }
}

/*
 * updateSession()
 *
 * Anytime the session data has changed, update the
 * session table to reflect the new data.
 */
void sessionDatabase::updateSession( void )
{
    caseInfo &info = caseInfo::Instance();
    errorHandler & err = errorHandler::Instance();
    QSqlQuery q;
    QSqlError sqlerr;

    // Note that there is no WHERE clause: all rows in session get updated.
    q.prepare( "UPDATE session SET notes = :notes, doctor = :doctor, location = :location, patient = :patient" );
    q.bindValue( ":notes",    info.getNotes() );
    q.bindValue( ":doctor",   info.getDoctor() );
    q.bindValue( ":location", info.getLocation() );
    q.bindValue( ":patient",  info.getPatientID() );
    q.exec();

    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to UPDATE session data: " ).append( sqlerr.databaseText() ) );
    }
}

/*
 * markExitAsClean()
 *
 * Set the flag in the database that this session was shut down
 * cleanly. Used in homescreen to determine if movie integrity
 * might be compromised.
 */
void sessionDatabase::markExitAsClean( void )
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler & err = errorHandler::Instance();

    q.prepare( "UPDATE session SET cleanExit = :value" );
    q.bindValue( ":value", true );
    q.exec();

    sqlerr = q.lastError();
    if ( sqlerr.isValid() )
    {
        err.warn( QObject::tr("Database failure:\nFailed to UPDATE clean exit flag." ) );
    }
}

/*
 * addCapture
 */
int sessionDatabase::addCapture( QString tag,
                                 int     timestamp,
                                 QString name,
                                 QString deviceName,
                                 bool    isHighSpeed,
                                 int     pixelsPerMm )
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler & err = errorHandler::Instance();
    QDateTime timeVal = QDateTime::fromTime_t(timestamp);
    QString timeStr = timeVal.toUTC().toString( "yyyy-MM-dd HH:mm:ss" );

    // Find next available ID
    int maxID = 0;
    q.prepare( "SELECT MAX(id) FROM captures" );
    q.exec();
    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to find MAX ID." ) );
        return -1;
    }
    q.next();
    QSqlRecord result = q.record();
    if( result.isNull("MAX(id)") )
    {
        maxID = 1;
    }
    else
    {
        int idCol = result.indexOf( "MAX(id)" );
        maxID = result.value( idCol ).toInt() + 1;
    }

    q.prepare( "INSERT INTO captures (id, timestamp, tag, name, deviceName, isHighSpeed, pixelsPerMm)"
               "VALUES (?, ?, ?, ?, ?, ?, ?)" );
    q.addBindValue( maxID );
    q.addBindValue( timeStr );
    q.addBindValue( tag) ;
    q.addBindValue( name );
    q.addBindValue( deviceName );
    q.addBindValue( isHighSpeed );
    q.addBindValue( pixelsPerMm );
    q.exec();

    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to INSERT new capture." ) );
        return -1;
    }

    return maxID;
}

/*
 * addClipCapture
 */
int sessionDatabase::addClipCapture( QString name,
                                     int     timestamp,
                                     QString catheterView,
                                     QString deviceName,
                                     bool    isHighSpeed )
{
    QSqlQuery q;
    errorHandler & err = errorHandler::Instance();
    QSqlError sqlerr;
    QDateTime timeVal = QDateTime::fromTime_t(timestamp);
    QString timeStr = timeVal.toUTC().toString("yyyy-MM-dd HH:mm:ss");

    // Find next available ID
    int maxID = 0;
    q.prepare( "SELECT MAX(id) FROM octLoops" );
    q.exec();
    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to find MAX ID." ) );
        return -1;
    }
    q.next();
    QSqlRecord result = q.record();
    if( result.isNull( "MAX(id)" ) )
    {
        maxID = 1;
    }
    else
    {
        int idCol = result.indexOf( "MAX(id)" );
        maxID = result.value( idCol ).toInt() + 1;
    }

    q.prepare( "INSERT INTO octLoops (id, timestamp, tag, name, length_ms, catheterView, deviceName, isHighSpeed)"
               "VALUES (?, ?, ?, ?, ? ,?, ?, ?)" );
    q.addBindValue( maxID );
    q.addBindValue( timeStr );
    q.addBindValue( name );         // Duplicate the name in the tag column by default
    q.addBindValue( name );         // file name without extension
    q.addBindValue( 0 );            // placeholder until the loop recording stops
    q.addBindValue( catheterView );
    q.addBindValue( deviceName );
    q.addBindValue( isHighSpeed );
    q.exec();

    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to INSERT new clip capture." ) );
        return -1;
    }

    return maxID;
}

/*
 * updateClipCapture
 *
 * Update the recording length of the OCT Loop after recording is finished.
 */
void sessionDatabase::updateClipCapture( int lastClipID, int clipLength_ms )
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler &err = errorHandler::Instance();

    q.prepare( "UPDATE octLoops SET id = :id, length_ms = :value WHERE id = :idToUpdate" );
    q.bindValue( ":id", lastClipID );
    q.bindValue( ":value", clipLength_ms );
    q.bindValue( ":idToUpdate", lastClipID );
    q.exec();

    sqlerr = q.lastError();
    if ( sqlerr.isValid() )
    {
        err.warn( QObject::tr("Database failure:\nFailed to UPDATE clip length." ) );
    }
}

/*
 * updateCaptureTag
 * 
 * Updates the stored tag for a given capture in the session database.
 */
void sessionDatabase::updateCaptureTag( int key, QString newTag )
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler &err = errorHandler::Instance();

    q.prepare( "UPDATE captures SET id = :id, tag = :value WHERE id = :idToUpdate" );
    q.bindValue( ":id", key );
    q.bindValue( ":value", newTag );
    q.bindValue( ":idToUpdate", key );
    q.exec();

    sqlerr = q.lastError();
    if ( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to UPDATE capture tag." ) );
    }
}


/*
 * updateLoopTag
 *
 * Updates the stored tag for a given capture in the session database.
 */
void sessionDatabase::updateLoopTag( int key, QString newTag )
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler &err = errorHandler::Instance();

    q.prepare( "UPDATE octLoops SET id = :id, tag = :value WHERE id = :idToUpdate" );
    q.bindValue( ":id", key );
    q.bindValue( ":value", newTag );
    q.bindValue( ":idToUpdate", key );
    q.exec();

    sqlerr = q.lastError();
    if ( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to UPDATE loop tag." ) );
    }
}

/*
 * getNumCaptures
 *
 * Query captures table in the session database.
 */
int sessionDatabase::getNumCaptures()
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler &err = errorHandler::Instance();

    q.prepare( "SELECT * from CAPTURES" );

   // Select count (*) is more straightforward, but QT does not seem to have the API to get the result.
   // q.prepare( "SELECT COUNT (*) from CAPTURES" );
    q.exec();

    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to query captures." ) );
    }

    int numCaptures = 0;
    while( q.next() )
    {
        numCaptures++;
    }
    qDebug() << "Total number of captures: " << numCaptures;

    return numCaptures;
}

/*
 * GetLoopsStats
 *
 * Query octloops table to get statistics in the session database.
 */
sessionDatabase::LoopStat sessionDatabase::getLoopsStats()
{
    QSqlQuery q;
    QSqlError sqlerr;
    errorHandler &err = errorHandler::Instance();

    q.prepare( "SELECT * from OCTLOOPS" );
    q.exec();

    sqlerr = q.lastError();
    if( sqlerr.isValid() )
    {
        err.warn( QObject::tr( "Database failure:\nFailed to query OCTLoops." ) );
    }

    QSqlRecord rec;
    rec = q.record();

    LoopStat loopStat;
    loopStat.numLoops = 0;
    loopStat.totLength = 0;

    int nameCol = rec.indexOf( "length_ms" );
    while( q.next() )
    {
         loopStat.numLoops++;
         loopStat.totLength += q.value( nameCol ).toInt();
    }

    qDebug() << "Total number of loops: " << loopStat.numLoops;
    qDebug() << "Total loop time (ms): " << loopStat.totLength;

    return loopStat;
}
