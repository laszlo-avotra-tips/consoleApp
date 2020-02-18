/*
 * initialization.cpp
 *
 * Runs required checks before the main application starts
 * to make sure the system is ready to run.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include "initialization.h"
#include "sawFile.h"
#include "defaults.h"
#include "buildflags.h"
#include "keys.h"
#include "logger.h"
#include "ioController.h"
#include "sledsupport.h"
#include "windowmanager.h"

/*
 * Constructor
 */
Initialization::Initialization()
{
    hasWarning         = false;
    runExeCheck        = true;
    docScreenAvailable = true;  // assume it is unless the check in init() fails
    statusMessage      = "";
}

/*
 * init
 *
 * Performs all necessary tests to make sure the system is ready to run; if
 * any tests fail, the application will report the failure and shutdown.
 */
bool Initialization::init( int argc, char *argv[] )
{
    bool isReady = true;

    qDebug() << "\n\n-------------------------------------";

    // Check that the application has at least this version of Qt libraries installed.
    // If this test fails, a pop-up happens automatically and the application quits.
    LOG( INFO, QString( "Qt Version: %1" ).arg( qVersion() ) )
    qDebug() << QString( "Qt Version: %1" ).arg( qVersion() );

//    QT_REQUIRE_VERSION( argc, argv, "5.12.6" );

    // Verify that an application instance has been created before running these
    // checks.  The hard drive space check and the pop-up at the end need the
    // instance to exist even though they do not access it directly.  Getting
    // rid of it immediately prevents the compiler from complaining about an
    // unused local variable.
    if ( !qApp )
    {
        QApplication *tmp = new QApplication( argc, argv );
        if( tmp )
        {
            delete tmp;
        }
    }

    // Verify the EXE is the one that was installed and has not been modified or
    // tampered with. This check only occurs in the release version but can be
    // overridden with a command line argument.
    bool isExeOk = false;

#if _DEBUG
    isExeOk = true;
#else
    if( runExeCheck )
    {
        // Verify the EXE is installed in the correct location
        const QString applicationDirPath = QCoreApplication::applicationDirPath();
        isExeOk = ( applicationDirPath == ExpectedApplicationDirPath );

        // Verify the EXE matches the digital signature that it was installed with
        Keys exeKey( applicationDirPath + "/octConsole.key", Keys::ReadOnly );
        exeKey.init();
        isExeOk = isExeOk && exeKey.isValid();
    }
    else
    {
        // overridden from the command line
        isExeOk = true;
    }
#endif

    ioController &ioc = ioController::Instance();

    SledSupport &ss = SledSupport::Instance();

    // Check Hardware, drivers and libraries
    if( !isExeOk )
    {
        statusMessage = tr( "ERROR: The executable file has been tampered with or\n" \
                            "       is not the original version that was installed." );
        isReady = false;
    }
//lcv
//    else if( !DAQ::isDaqPresent() )
//    {
//        statusMessage = tr( "ERROR: No DAQ Hardware found." );
//        isReady = false;
//    }
//    else if( !DAQ::checkSDKVersion() )
//    {
//        statusMessage = tr( "ERROR: DAQ SDK Version mismatch." );
//        isReady = false;
//    }
//    else if( !DAQ::checkDriverVersion() )
//    {
//        statusMessage = tr( "ERROR: DAQ Driver Version mismatch." );
//        isReady = false;
//    }
    else if( !ioc.queryDevice() )
    {
        statusMessage = tr( "ERROR: Unable to communicate with the USB DAC hardware." );
        isReady = false;
    }
    else if( !ss.init() )
    {
        statusMessage = tr( "ERROR: Unable to communicate with Sled Support Board hardware." );
        isReady = false;
    }
    else
    {
        // Check that the Physician monitor is present, otherwise the process will hang
        qDebug() << "Initialization::init - checking for Physician monitor";
        WindowManager &wm = WindowManager::Instance();
        wm.init();
        qDebug() << "wm.isPhysicianMonPresent() =" << wm.isPhysicianMonPresent();
        if( !wm.isPhysicianMonPresent()  )
        {
#if ENABLE_SINGLE_MONITOR_WARNING
            statusMessage = QString( tr( "Physician monitor not found. Check video and power cables and that the monitor is ON." ) );
            isReady = false;
#endif
            docScreenAvailable = false;
        }
    }

    // Make sure the Data directory exists. If not, make it so.
    QDir dataDir( DataDir );
    if( !dataDir.exists() )
    {
        if( !dataDir.mkpath( DataDir ) )
        {
            statusMessage = tr( "ERROR: The data directory (%1) could not be created." ).arg( DataDir );
            isReady = false;
        }
    }

    // Check for the System directory
    QDir sysDir( SystemDir );

    // set flags for use below
    bool sysKeysAreOk = false;

#if ENABLE_SYSTEM_KEY_CHECK
    // If the system directory exists, check if the keys exist and are valid
    if( sysDir.exists() )
    {
        // Load the system keys file and check to make sure the data files have not been tampered with
        Keys k( SystemKeysPath, Keys::ReadOnly );
        k.init();

        sysKeysAreOk = k.isValid();

        // Check for required files in the key file
        QStringList requiredFiles;
        requiredFiles << "octConsole.ini"
                      << "RescalingData.csv";

        if( !k.containsRequiredFiles( requiredFiles ) )
        {
            statusMessage = tr( "ERROR: Required files are missing in %1." ).arg( SystemKeysPath );
            isReady = false;
        }
    }
#else
    // Always OK in devel
    sysKeysAreOk = true;
    qDebug() << "System Key Check is disabled";
#endif

    // If the system keys are not good, set the error
    if( !sysKeysAreOk ) //lcv
    {
        statusMessage = tr( "ERROR: System key mismatch in %1." ).arg( SystemKeysPath );
        isReady = false;
    }

    // Send message to debug system. Failures are handled by the caller
    if( isReady )
    {
        qDebug() << "Initialization: Good to go!";
    }

    return isReady;
}
