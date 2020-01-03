/*
 * eventData.cpp
 *
 * Handle saving event data to disk. These events are case-specific and are
 * stored in the case directory.
 *
 * Author(s): Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */

#include <QDebug>
#include "defaults.h"
#include "eventDataLog.h"
#include "logger.h"
#include "Utility/userSettings.h"
#include "util.h"

/*
 * Constructor
 *
 * Open the event log file for writing. Open in append mode in case the
 * application is recovering from a crash.
 */
EventDataLog::EventDataLog()
{
    hFile     = nullptr;
    xmlWriter = nullptr;
}

/*
 * Destructor
 *
 * Flush the stream contents and close the file.
 */
EventDataLog::~EventDataLog()
{
    delete xmlWriter;
    delete hFile;
}

/*
 * init
 *
 * Initialize the log for writing.  Create and open the file. Start the XML
 * handler.
 */
void EventDataLog::init( void )
{
    errorHandler & err = errorHandler::Instance();

    // Get access to the storage directory
    caseInfo &caseInfo = caseInfo::Instance();

    const QString EventLogFile = caseInfo.getStorageDir() + "/" + EventDataLogFileName;
    hFile = new QFile( EventLogFile );

    if( !hFile )
    {
        // fatal error
        err.fail( QObject::tr( "Could not get a file handle for the Event Data Log for this session." ), true );
    }

    // Open the file for writing
    if( !hFile->open( QFile::WriteOnly | QFile::Append | QFile::Text ) )
    {
        // fatal error
        err.fail( QString( QObject::tr( "Could not open %1 for writing." ) ).arg( EventLogFile ), true );
    }
    else
    {
        xmlWriter = new QXmlStreamWriter( hFile );
        
        if( !xmlWriter )
        {
            // fatal error
            err.fail( QObject::tr( "Could not create the handle for the Event Data Log XML file." ), true );
        }
        
        xmlWriter->setAutoFormatting( true );
        xmlWriter->writeStartDocument();
        xmlWriter->writeStartElement( "Events" );
    }
}


/*
 * addEvent
 *
 * Adds the event information to the file
 *
 */
void EventDataLog::addEvent( QString eventName,
                             unsigned long frameCount,
                             unsigned long timeStamp,
                             QString dataFileName )
{
    // Add the event to the file
    xmlWriter->writeStartElement( "event" );
    xmlWriter->writeAttribute( "name", eventName );
    xmlWriter->writeAttribute( "frameCount", QString::number( frameCount ) );
    xmlWriter->writeAttribute( "timeStamp", QString::number( timeStamp ) );
    xmlWriter->writeAttribute( "dataFile", dataFileName );
    xmlWriter->writeEndElement();
    hFile->flush();
}

/*
 * close
 *
 * Close out the XML data and close the file
 */
void EventDataLog::close( void )
{
    xmlWriter->writeEndDocument();
    hFile->close();
}
