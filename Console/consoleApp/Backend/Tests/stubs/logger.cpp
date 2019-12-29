/*
 * logger.cpp
 *
 * Stubs for unit tests that call the logger.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010 Avinger, Inc.
 *
 */
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include "logger.h"
#include "defaults.h"

/*
 * Constructor
 *
 */
Logger::Logger()
{
}

/*
 * Destructor
 *
 * Flush the stream contents and close the file.
 */
Logger::~Logger()
{

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
bool Logger::init( void )
{
    bool isOk = true;

    return isOk;
}

/*
 * logMessage
 *
 * Write messages along with a timestamp and other details to the system log file. A
 * mutex is used to prevent multiple threads from colliding.
 */
void Logger::logMessage( QString /*msg*/, const char * /*severity*/, const char * /*file */, int /*line*/ )
{

}
