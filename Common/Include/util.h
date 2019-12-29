/*
 * util.h
 *
 * Various utility functions and defines that are needed
 * by both the Frontend and Backend.
 *
 * Author: Chris White, Dennis W. Jackson
 *
 * Copyright (c) 2010-2017 Avinger, Inc.
 *
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <QString>
//lcv
//#ifndef WIN32
//#include <math.h>
//#endif

#include <QObject>
#include <QDir>

inline int floor_int (float x)
{
//#ifdef WIN32
//    const float    round_towards_m_i = -0.5f;
//    int            i;
//    __asm
//    {
//        fld   x
//        fadd  st, st (0)
//        fadd  round_towards_m_i
//        fistp i
//        sar   i, 1
//    }
//    return (i);
//#else
//    return ( floor( x ) );
//#endif
    return int(x);
}

// Get the sign of a value
template <typename T>
T sign(T t)
{
    return ( ( t == 0 ) ? T(0) : ( (t < 0) ? T(-1) : T(1) ) );
}

#undef max
#undef min
template <typename T>
T max( T a, T b )
{
    return( a > b ? a : b );
}

template <typename T>
T min( T a, T b )
{
    return( a < b ? a : b );
}

// Math defines
const double pi(3.1415);
const double degToRad(pi/180.0);

// Conversion constants
const unsigned long  B_per_KB( 1024 );
const unsigned long KB_per_MB( 1024 );
const unsigned long MB_per_GB( 1024 );
const unsigned long  B_per_GB( B_per_KB * ( KB_per_MB * MB_per_GB ) );

// General-use functions to pop up a message to the user:
//    Warnings are benign
//    Failures are fatal
//  This class is a convenience for threaded objects and entities
//  that are not otherwise connected to the GUI thread to issue
//  warnings. Other clients can use the utility functions below
//  directly.
class errorHandler : public QObject
{
    Q_OBJECT

public:

    static errorHandler & Instance(void) {
        static errorHandler theHandler;
        return theHandler;
    }

    void fail( QString errString, bool fatal = true ) {
        emit failure( errString, fatal );
    }

    void warn( QString warnString ) {
        emit warning( warnString );
    }

signals:
    void failure( QString, bool );
    void warning( QString );

private:
    errorHandler() {}
    ~errorHandler() {}

    errorHandler( errorHandler const & ); // hide copy
    errorHandler & operator=( errorHandler const & ); // hide assign

};

void displayFailureMessage( QString errString, bool fatal );
void displayWarningMessage( QString warnString );

/*
 * Print "text" to debug output once per second assuming DEBUG_FPS frames/s
 *
 * "text" can be strings, variables, commands, etc.
 * e.g., DEBUG_PRINT_AT_FRAME_RATE( "DDC: linesPerRevolution = " << linesPerRevolution )
 */
#define DEBUG_FPS (17)
#define DEBUG_PRINT_AT_FRAME_RATE( txt ) \
{ \
    static int tt = 0; \
    if( ++tt == DEBUG_FPS ) \
    { \
        tt = 0; \
        qDebug() << QTime::currentTime() << txt; \
    } \
}

struct SystemData_T
{
    QString softwareVersionNumber;
    QString kioskVersionNumber;
    QString consoleVersionNumber;
    QString laserSerialNumber;
};

// Device specific funtions used by HomeScreen and octConsole
QString getSoftwareVersionNumber( void );
QString getConsoleSerialNumber( void );
QString getLaserSerialNumber( void );
QString getKioskVersionNumber( void );
QString getTrainingVersionNumber( void );
QString getTextVersionNumber( const QString CookieFileName, const QString VersionPrefixString );

SystemData_T getSystemData( void );

qint64 getDirectorySize_B( QDir directory );

void updateSessionCookieFile( QString caseID );

#endif  // UTIL_H_
