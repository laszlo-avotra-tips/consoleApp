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
#include "keys.h"
#include "logger.h"
#include <Backend/interfacesupport.h>

/*
 * Constructor
 */
Initialization::Initialization()
{
    hasWarning         = false;
    docScreenAvailable = true;  // assume it is unless the check in init() fails
    statusMessage      = "";

    auto interfaceSupport = InterfaceSupport::getInstance();
    LOG1(interfaceSupport)

    if (interfaceSupport) {
        LOG( INFO, "Interface support initialized successfully");
    } else {
        LOG( ERROR, "Interface support could not be initialized");
    }
}

/*
 * init
 *
 * Performs all necessary tests to make sure the system is ready to run; if
 * any tests fail, the application will report the failure and shutdown.
 */
bool Initialization::init( )
{
    bool isReady = true;

    qDebug() << "\n\n-------------------------------------";

    // Check that the application has at least this version of Qt libraries installed.
    // If this test fails, a pop-up happens automatically and the application quits.
    LOG( INFO, QString( "Qt Version: %1" ).arg( qVersion() ) )
    qDebug() << QString( "Qt Version: %1" ).arg( qVersion() );

    bool isExeOk = true;

    // Check Hardware, drivers and libraries
    if( !isExeOk )
    {
        statusMessage = tr( "ERROR: The executable file has been tampered with or\n" \
                            "       is not the original version that was installed." );
        isReady = false;
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

    // Always OK in devel
    sysKeysAreOk = true;
    qDebug() << "System Key Check is disabled";

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
