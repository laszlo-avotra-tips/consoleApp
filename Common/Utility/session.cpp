/*
 * session.cpp
 *
 * Create and maintain case session storage information.
 *
 * Author(s): Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QDateTime>
#include <QObject>  // defines connect()
#include "defaults.h"
#include "session.h"
#include "Utility/userSettings.h"
#include "logger.h"

/*
 * Constructor
 */
Session::Session()
{
    // no unknown pointers
    eventLog = nullptr;
    keys     = nullptr;
}

/*
 * init
 */
void Session::init( void )
{
    caseInfo &info = caseInfo::Instance();
    // Check that a proper storage dir exists before trying to access it.
    if( info.storageValid() )
    {
        const QString StorageDir = info.getStorageDir();

        // create session files
        mutex.lock();
        eventLog = new EventDataLog();
        mutex.unlock();
        keys     = new Keys( StorageDir + "/" + KeyFile, Keys::WriteOnly );

        if( !eventLog || !keys )
        {
            // Fatal error
            emit sendError( "Session::init() failed." );
        }
        else
        {
            // Set up the key file for this session
            keys->init();

            // handle adding files to the key hash
            QObject::connect( keys, SIGNAL(sendError(QString)),   this, SIGNAL(sendError(QString)) );
            QObject::connect( keys, SIGNAL(sendWarning(QString)), this, SIGNAL(sendWarning(QString)) );

            mutex.lock();
            eventLog->init();
            mutex.unlock();
        }
    }
    else
    {
        emit sendError( "Session::init() failed.\nCase Storage Dir not set." );
    }

}

/*
 * Destructor
 *
 * Release memory, stop recording data and close out the files
 */
Session::~Session()
{
}

/*
 * start
 *
 * Start the session by logging an event to the log
 */
void Session::start( void )
{
    caseInfo &info = caseInfo::Instance();

    int timeOffset = info.getUtcOffset();
    QString eventString = QString( "Session Start UTC Offset:%1" ).arg( timeOffset );
    sendSessionEvent( eventString );
    LOG( INFO, eventString )
}

/*
 * shutdown
 *
 * Shutdown the session by logging an event to the log, keying the files, and 
 * releasing those file handles.
 */
void Session::shutdown( void )
{
    // Add a line to the event log
    emit sendSessionEvent( "Session End" );

    if( eventLog )
    {
        mutex.lock();
        eventLog->close();
        mutex.unlock();
    }

    if( keys )
    {
        // Add session-specific files to the key list before shutting down
        // the key object.
        keys->addFile( EventDataLogFileName );
        keys->addFile( SessionDatabaseFileName );
        delete keys;
        keys = nullptr;
    }
}

/*
 * handleFileToKey
 *
 * The parameter filename can include the path to the file (relative or absolute)
 *
 */
void Session::handleFileToKey( QString filename )
{
    if( keys )
    {
        keys->addFile( filename );
    }
}
