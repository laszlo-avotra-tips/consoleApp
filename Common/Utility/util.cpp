/*
 * util.cpp
 *
 * Various utility functions and defines that are needed
 * by both the Frontend and Backend.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */

//#include <QtGui/QApplication>
#include <QApplication>
#include "styledmessagebox.h"
#include <QThread>
#include <QDebug>
#include <QSettings>
#include <QString>
#include "logger.h"
#include "util.h"
#include "defaults.h"
#include "version.h"
#include "buildflags.h"
//#include "laser.h"

/*
 * displayFailureMessage
 *
 * Handle any failures. The message is logged if the logging
 * system is functional and posted to the user.  The application shuts down
 * after the user dismisses this dialog. Since this calls for a
 * GUI based message box, this can only be safely called from the main thread.
 */
void displayFailureMessage( QString errString, bool fatal )
{
    // Attempt to write to the log file.  If the log is the cause of the failure
    // the message will only appear on the screen.
    LOG( INFO, "-------------------" )
    LOG( FATAL, errString )

//    Laser &laser = Laser::Instance();
//    laser.forceOff();

    // make sure this message is going out via the GUI thread.
    const bool IsGuiThread = ( QThread::currentThread() == QCoreApplication::instance()->thread() );

    QString s = QApplication::tr( errString.toLatin1() );

    s.append( QObject::tr( "\n\nContact Customer Service at " ).append( ServiceNumber ) );

    if( IsGuiThread )
    {
        // Pop-up a dialog box with the error. The application will stop.
        styledMessageBox::critical( QObject::tr( "An unrecoverable error has occurred; the software must terminate.\n" ).append( s ) );
    }
    else
    {
        // Log a message for development testing
        qDebug() << "displayFailureMessage() was called from the non-GUI thread.";
        LOG( DEBUG, "displayFailureMessage() was called from the non-GUI thread.  " )
    }

    if ( fatal )
    {
        qFatal( s.toLatin1().data() );
    }
}


/*
 * displayWarningMessage
 *
 * Handle any warnings. The message is logged if the logging
 * system is functional and posted to the user. Since this
 * calls for a GUI based message box, this can only safely
 * be called from the main thread.
 */
void displayWarningMessage( QString warnString )
{
    // make sure this message is going out via the GUI thread.
    const bool IsGuiThread = ( QThread::currentThread() == QCoreApplication::instance()->thread() );

    if( IsGuiThread )
    {
        styledMessageBox::warning( QObject::tr( "Attention:\n" ).append( QObject::tr( warnString.toLatin1() ) ) ); // Calls LOG inside warning()
    }
    else
    {
        LOG( WARNING, warnString.prepend( "Called from the non-GUI thread. ") )
    }
}

/*
 * getSoftwareVersionString
 *
 * Return the software version as a string
 */
QString getSoftwareVersionNumber()
{
    return( QString( "%1.%2.%3" ).arg( C_MAJOR_VERSION ).arg( C_MINOR_VERSION ).arg( C_PATCH_VERSION ) );
}


/*
 * getSerialNumber
 *
 * Return the system serial number as stored in the system configuration file
 *
 */
QString getConsoleSerialNumber( void )
{
    QSettings consoleInfo( ConsoleIniFile, QSettings::IniFormat );
    return( consoleInfo.value( LightboxSerialNumberKey, DefaultLightboxSerialNumber ).toString() );
}

/*
 * getLaserNumber
 *
 * Return the laser serial number as stored in the system configuration file
 *
 */
QString getLaserSerialNumber( void )
{
    QSettings consoleInfo( ConsoleIniFile, QSettings::IniFormat );
    return( consoleInfo.value( LaserSerialNumberKey, DefaultLaserSerialNumber ).toString() );
}

/*
 * getKioskVersionNumber
 *
 * Return the text with the version number (or N/A) from the kiosk cookie file
 */
QString getKioskVersionNumber( void )
{
    return( getTextVersionNumber( KioskCookieFile, "Kiosk Setup ver: " ) );
}

/*
 * getTrainingVersionNumber
 *
 * Return the text with the version number (or N/A) from the Training cookie file
 */
QString getTrainingVersionNumber()
{
    return( getTextVersionNumber( TrainingCookieFile, "Training ver: " ) );
}

/*
 * getTextVersionNumber
 *
 * Handler for extracting version strings from files.
 */
QString getTextVersionNumber( const QString CookieFileName, const QString VersionPrefixString )
{
    /*
     * Version to display if the file is missing or unreadable.
     */
    QString verText = "N/A";

    /*
     * Try and extract the version from the file given.
     */
    if( QFile::exists( CookieFileName ) )
    {
        QFile *input = new QFile( CookieFileName );

        if( ( input ) && ( input->open( QIODevice::ReadOnly ) ) )
        {
            QTextStream in( input );

            verText = in.readLine();

            // Return just the version number
            verText = verText.remove( VersionPrefixString, Qt::CaseInsensitive ).trimmed();
        }

        // close the file
        input->close();
    }

    return( verText );
}

/*
 * getDirectorySize_B
 *
 * Give a QDir, iterate through all of it's files ( and subdirectories )
 * and add up the sizes.  Return size is in Bytes.
 */
qint64 getDirectorySize_B( QDir directory )
{
    QFileInfoList files = directory.entryInfoList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks );
    QFileInfo fi;
    qint64 size = 0;

    foreach( fi, files )
    {
        if( fi.isDir() )
        {
            // Recursion yay!
            size += getDirectorySize_B( QDir( fi.absoluteFilePath() ) );
        }
        else
        {
            size += fi.size();
        }
    }
    return size;
}

/*
 * updateSessionCookieFile
 *
 * Creates or appends to the database cookie file.  This file is used by HomeScreen
 * to add new cases to the master database.
 */
void updateSessionCookieFile( QString caseID )
{
    // Save case ID to the cookie file for updating the master database when the case ends
    QFile sessionCookieFile( CaseCookieFilename );

    if( !sessionCookieFile.open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text ) )
    {
        displayWarningMessage( QObject::tr( "Failed to create session cookie. You will need to manually refresh the case database." ) );
    }
    else
    {
        QTextStream output( &sessionCookieFile );
        output << caseID << endl;
    }
}

/*
 * getSystemData
 * 
 * Method to get all relevant system data into one structure
 */
SystemData_T getSystemData()
{
    SystemData_T data;

    data.consoleVersionNumber  = getConsoleSerialNumber();
    data.kioskVersionNumber    = getKioskVersionNumber();
    data.laserSerialNumber     = getLaserSerialNumber();
    data.softwareVersionNumber = getSoftwareVersionNumber();

    return data;
}
