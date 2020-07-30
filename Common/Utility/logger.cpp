/*
 * logger.cpp
 *
 * A common method for all classes to log data to disk in a thread-safe way.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include "logger.h"
#include "defaults.h"
#include "util.h"
#include "buildflags.h"
#include "version.h"

Logger* Logger::theLogger{nullptr};
// Constants
const int MaxLogSize_bytes = 2 * B_per_KB * KB_per_MB;

// Singleton
Logger & Logger::Instance() {
    if(!theLogger){
        theLogger = new Logger();
    }
    return *theLogger;
}

/*
 * Constructor
 *
 */
Logger::Logger()
{
    hFile   = nullptr;
    output  = nullptr;
    appName = "";
}

/*
 * Destructor
 *
 * Flush the stream contents and close the file.
 */
Logger::~Logger()
{
    if( hFile )
    {
        hFile->flush();
        hFile->close();
        delete hFile;
    }

    if( output )
    {
        delete output;
    }
}


/*
 * init
 *
 * Open the system log file for appending if exists; otherwise, it creates the
 * the system log file.
 *
 * Any errors are stored to allow the object that initializes the log to
 * determine what went wrong.  The log is typically created before the
 * rest of the system is initialized so there is no one to send signals to.
 */
bool Logger::init( QString applicationName )
{
    bool isOk = true;

    // Logs are saved in a hard-coded location    
    if( !getFileHandle( SystemLogFileName ) )
    {
        status = QObject::tr( "ERROR: Could not get file handle for " ) + SystemLogFileName;
        isOk = false;
    }
    else
    {
        if( !rotateLog( SystemLogFileName ) )
        {
            status = QObject::tr( "ERROR: Could not rotate the log file, " ) + SystemLogFileName;
            isOk = false;
        }
        else
        {
            // open or create the log file
            if( !hFile->open( QIODevice::Append | QIODevice::Text ) )
            {
                status = QObject::tr( "ERROR: Could not open %1 for appending." ).arg( SystemLogFileName );
                isOk = false;
            }
            else
            {
                output = new QTextStream( hFile );

                if( !output )
                {
                    status = QObject::tr( "ERROR: Could not open %1 for writing." ).arg( SystemLogFileName );
                    isOk = false;
                }
            }
        }
    }

    appName = applicationName;

    return isOk;
}

/*
 * close
 *
 * Close our file handle to the log. This is so that other clients
 * in other processes can access it later. Requires a new call to init()
 * if usage is to continue.
 */
void Logger::close( void )
{
    if (hFile) {
        hFile->close();
        hFile = nullptr;
    }
}

/*
 * getFileHandle
 *
 * Get and check a handle for the system log
 */
bool Logger::getFileHandle( const QString systemLogFileName )
{
    // set the file handle for the object
    hFile = new QFile( systemLogFileName );

    // return the status of the memory allocation
    return( hFile != nullptr );
}
  
/*
 * rotateLog
 *
 * Check the size of the log and rotate it if necessary.
 */
bool Logger::rotateLog( const QString systemLogFileName )
{
    bool isOk = true;

    // Check if the log file has grown large. If so, rename it and a new one
    // will be started.
    if( hFile->exists() && ( hFile->size() > MaxLogSize_bytes ) )
    {
        // create a unique name for the old log file
        QString timeStamp = QDateTime::currentDateTime().toUTC().toString( "yyyyMMdd-hhmmss" );
        QString newName   = SystemDir + "/OCT_System-" + timeStamp + ".log";
        hFile->rename( newName );

        // release the old handle
        delete hFile;

        // create a new handle.  Error is handled by calling function
        isOk = getFileHandle( systemLogFileName );
    }

    return isOk;
}

/*
 * logMessage
 *
 * Write messages along with a timestamp and other details to the system log file. A
 * mutex is used to prevent multiple threads from colliding.
 */
void Logger::logMessage( QString msg, const char *severity, const char *file, int line )
{
//    return; //lcv
    // Make file writing this thread-safe
    QMutexLocker locker( &mutex );

    // remove new line characters for easier parsing of the log.
    msg = msg.replace( "\n", ". " );

    // make sure we have a stream to write to
    if( output )
    {
        // write the UTC time-stamped log message and flush the file (endl)
        *output << "[" << QDateTime::currentDateTime().toUTC().toString( "yyyy-MM-dd HH:mm:ss.zzz" ) << "] "
                << "(" << appName << "." << C_PATCH_VERSION << ") "
                << severity << ": " 
                << msg.toLatin1() << " - "
                << file << " (" << line << ")"
                << endl;
    }

#if ENABLE_LOGGING_TO_DEBUG_WINDOW
    // Send all log messages to the debug window as well
    qDebug() << severity << msg.toLatin1();
#endif
}

void Logger::logDebugMessage(const QString &msg, const char* function, int line, Qt::HANDLE tId)
{
    QMutexLocker locker( &mutex );

    *output << "[" << QDateTime::currentDateTime().toUTC().toString( "yyyy-MM-dd HH:mm:ss.zzz" ) << "] "
            << "(" << appName << "." << C_PATCH_VERSION << ") DEBUG: THR(" << tId << ") "
            << " - " << function << " (" << line << ") -> "
            << msg << endl;
}

