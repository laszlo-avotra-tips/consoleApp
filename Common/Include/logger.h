/*
 * logger.h
 *
 * A simple method for all classes to log data to disk in a thread-safe
 * way. The file and line number of the message is stored with the timestamp
 * of the message. This implementation does not allow for levels of logging
 * to be turned off. An interesting artifact of the implementation allows
 * any text to be used as the "severity" and this will be logged.
 *
 * Standard Severities:
 *    DEBUG:   information for developers about the state of the application
 *    FATAL:   informations about what caused the application to shut down
 *    INFO:    information about the state of the application
 *    WARNING: information that was displayed to the user
 *
 * To use:
 *   #include "logger.h"
 *   LOG( INFO, "text to log" );
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2017 Avinger, Inc.
 *
 */
#ifndef LOGGER_H_
#define LOGGER_H_

#include <QFile>
#include <QMutexLocker>  // make file writes thread-safe
#include <QString>
#include <QTextStream>
#include <QThread>

// Macro for all classes to use for logging
//#if UNIT_TEST
//   #define LOG( severity, msg ) {}
//#else
   #define LOG( severity_, msg_ ) { Logger &log_ = Logger::Instance(); \
                                  log_.logMessage( msg_, #severity_, __FILE__, __LINE__ ); }
//#endif  // UNIT_TEST


#define LOG1(var_) { Logger &log = Logger::Instance(); \
    QString msg; QTextStream qts(&msg); \
    qts << #var_ << "=" << var_ << endl; \
    log.logDebugMessage(msg, __FUNCTION__, __LINE__, QThread::currentThreadId());}

#define LOG2(x_,y_) { Logger &log = Logger::Instance(); \
    QString msg; QTextStream qts(&msg); \
    qts << #x_ << "=" << x_ << " " << #y_ << "=" << y_ << endl; \
    log.logDebugMessage(msg, __FUNCTION__, __LINE__, QThread::currentThreadId());}

#define LOG3(x_,y_,z_) { Logger &log = Logger::Instance(); \
    QString msg; QTextStream qts(&msg); \
    qts << #x_ << "=" << x_ << " " << #y_ << "=" << y_ << " " << #z_ << "=" << z_ << endl; \
    log.logDebugMessage(msg, __FUNCTION__, __LINE__, QThread::currentThreadId());}

#define LOG4(x_,y_,z_,zz_) { Logger &log = Logger::Instance(); \
    QString msg; QTextStream qts(&msg); \
    qts << #x_ << "=" << x_ << " " << #y_ << "=" << y_ << " " << #z_ << "=" << z_ << " " << #zz_ << "=" << zz_ << endl; \
    log.logDebugMessage(msg, __FUNCTION__, __LINE__, QThread::currentThreadId());}

class Logger
{
public:
    // Singleton
    static Logger & Instance();
    bool init( QString applicationName );
    void close( void );

    QString getStatusMessage( void ) { return status; }

    void logMessage( QString msg, const char *severity, const char* file, int line );
    void logDebugMessage(const QString& msg, const char* function, int line, Qt::HANDLE tId );
    QTextStream& getTextStream(const char* function, int line, Qt::HANDLE tId );

private:
    Logger();  // hide ctor
    ~Logger(); // hide dtor
    Logger( Logger const & ); // hide copy
    Logger & operator=( Logger const & ); // hide assign

    bool getFileHandle(const QString systemLogFileName );
    bool rotateLog(const QString systemLogFileName );

    QFile *hFile;
    QTextStream *output;

    QMutex mutex;

    QString status;
    QString appName;

     static Logger* theLogger;
};

#endif  // LOGGER_H_
