/*
 * main.cpp
 *
 * The OCT Console application
 *
 * Author: Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QObject>
#include <QCommandLineOption>
#include "qtsingleapplication.h"
#include "caseinfowizard.h"
#include "Screens/frontend.h"
#include "buildflags.h"
#include "initialization.h"
#include "laser.h"
#include "logger.h"
#include "util.h"
#include "version.h"
#include "Utility/sessiondatabase.h"
#include "powerdistributionboard.h"
#include "styledmessagebox.h"
#include "util.h"
#include "keyboardinputcontext.h"
#include "sledsupport.h"
#include "daqfactory.h"
#include "signalmanager.h"

void parseOptions( QCommandLineOption &options, QStringList args );

/*
 * parseOptions
 *
 * Set up and handle command line options
 */
void parseOptions( QCommandLineOption &options, QStringList args )
{
    LOG2(&options,args.size())
    // make options unix-like
//    options.setFlagStyle( QxtCommandOptions::DoubleDash );
//    options.setParamStyle( QxtCommandOptions::SpaceAndEquals );

//    options.add( "noexe", "Disable key checks for the executable" );
//    options.add( "port", "Assign the COM port to use for controlling the laser (e.g., -p COM3)", QxtCommandOptions::Required );
//    options.alias( "port", "p" );
//    options.add( "low-space", "Run in low drive space mode (no captures or recording)");

//    options.parse( QCoreApplication::arguments() );

//    // Log any command line options. Qt pulls out any Qt-only arguments before this point
//    // The program name is always passed inF
//    if( args.size() > 1 )
//    {
//        // Log any arguments
//        for (int i = 0; i < args.size(); ++i)
//        {
//            LOG( INFO, QString( "Command line arguments: %1" ).arg( args.at( i ) ) );
//        }
//    }
}


/*
 * main
 */
int main(int argc, char *argv[])
{
    // Only allow a single instance of the program to run
    QtSingleApplication app( "OCT Console", argc, argv );

    // use the touch keyboard
//    keyboardInputContext *ic = new keyboardInputContext();
//    app.setInputContext( ic );

//    if( app.isRunning() )
//    {
//        return 0;
//    }

    // single application initialization
    app.initialize();

    // Set-up the logging system and make sure it is OK to run
    Logger &log = Logger::Instance();
    if( !log.init( "consoleApp" ) )
    {
        displayFailureMessage( log.getStatusMessage(), true );
    }

    // Start the session in the system log
    LOG( INFO, "-------------------" )
    LOG( INFO, "Application started: OCT HS Console" )
    LOG( INFO, QString( "OCT Console Process ID (PID) : %1" ).arg( app.applicationPid() ) )
    LOG( INFO, QString( "OCT Console Version: %1" ).arg( getSoftwareVersionNumber() ) )

#if _DEBUG
    LOG( INFO, "DEBUG Build" )
#else
    LOG( INFO, "RELEASE Build" );
#endif

    LOG( INFO, QString( "Local time is %1" ).arg( QDateTime::currentDateTime().toString( "yyyy-MM-dd HH:mm:ss" ) ) )

//lcv
    // check for command line options and use them if they are present
//    QCommandLineOption options("");
//    QStringList args = app.arguments();
//    parseOptions( options, args );

    // Check the EXE key?  Default to true
//    bool runExeCheck = !options.count( "noexe" );
    bool runExeCheck = true;

#if !ENABLE_EXE_CHECKS
    runExeCheck = false;
#endif

    // Run start-up checks
    Initialization init;

    init.setExeCheck( runExeCheck );

//#if USE_INIT
//    if( !init.init( argc, argv ) )
//    {
//        // clean up if init checks fail
//        if( ic )
//        {
//            delete ic;
//        }
//        displayFailureMessage( init.getStatusMessage(), true );
//    }

//    if( init.warningPosted() )
//    {
//        displayWarningMessage( init.getStatusMessage() );
//    }
//#endif

//#ifdef  QT_NO_DEBUG
//    // Kick off a background thread to run additional start-up functions
//    init.start();
//#endif
    // create the main window
    frontend frontEndWindow;

//#if QT_NO_DEBUG
//    // Provide power to all other non-PC components in the Lightbox
//    powerDistributionBoard pdb;
//    pdb.powerOn();
//#endif

    // if both monitors are not present, only show the technician's
    if( !init.isPhysicianScreenAvailable() )
    {
        frontEndWindow.turnOffPhysicianScreen();
    }

    app.setActivationWindow( &frontEndWindow );

    // application return status
    int status = 0;

    // Initialize frontend processes
    frontEndWindow.init();

    SignalManager::instance();

    // Select a device and initialize data acquisition
    int result = frontEndWindow.setupCase( true );

    if( result == QDialog::Accepted )
    {
        auto idaq = daqfactory::instance()->getdaq();
        frontEndWindow.setIDAQ(idaq);

        QObject::connect( &app, SIGNAL( aboutToQuit() ), &frontEndWindow, SLOT( shutdownCleanup() ) );

//#if QT_NO_DEBUG
//        Laser &laser = Laser::Instance();

//        // default serial port
//        QString portName = DefaultPortName;
//        if( options.count( "port" ) )
//        {
//            portName = options.value( "port" ).toString();
//        }

//        laser.setPort( portName.toLatin1() );

//        QString laserCommConfig = DefaultLaserCommConfig;
//        laser.setConfig( laserCommConfig.toLatin1() );

//        QObject::connect( &frontEndWindow,      SIGNAL(checkLaserDiodeStatus()), &laser, SLOT(isDiodeOn()) );
//        QObject::connect( &laser,  SIGNAL(diodeIsOn(bool)),         &frontEndWindow,     SIGNAL(forwardLaserDiodeStatus(bool)) );
//        QObject::connect( &frontEndWindow,      SIGNAL(forwardTurnDiodeOn()),    &laser, SLOT(turnDiodeOn()) );
//        QObject::connect( &frontEndWindow,      SIGNAL(forwardTurnDiodeOff()),   &laser, SLOT(turnDiodeOff()) );

//        // initialize the hardware for communicating to the laser
//        laser.init();
//#else
        LOG( INFO, "LASER: serial port control is DISABLED" )
//#endif

//#if QT_NO_DEBUG
//        SledSupport &sledSupport = SledSupport::Instance();
//        QObject::connect( &sledSupport, SIGNAL( announceClockingMode( int ) ),
//                          &frontEndWindow,           SIGNAL( announceClockingMode( int ) ) );
//        QObject::connect( &sledSupport, SIGNAL( announceFirmwareVersions( QByteArray, QByteArray ) ),
//                          &frontEndWindow,           SIGNAL( announceFirmwareVersions( QByteArray, QByteArray ) ) );
//        QObject::connect( &frontEndWindow,           SIGNAL( updateDeviceForSledSupport() ),
//                          &sledSupport, SLOT(   updateDeviceForSledSupport() ) );
//        QObject::connect( &sledSupport, SIGNAL( changeDeviceSpeed( int, int ) ),
//                          &frontEndWindow,           SLOT(   changeDeviceSpeed( int, int ) ) );
//        QObject::connect( &sledSupport, SIGNAL( handleError(QString ) ),
//                          &frontEndWindow,           SLOT(   handleError(QString) ) );
//        QObject::connect( &sledSupport, SIGNAL( setDirButton( int ) ),
//                          &frontEndWindow,           SLOT(   dirButton( int ) ) );
//
//        QObject::connect( &frontEndWindow, SIGNAL( checkSledStatus() ), &sledSupport, SLOT( getAllStatus() ) );
//
//#else // !QT_NO_DEBUG
        LOG( INFO, "SLED support board: serial port control is DISABLED" )
//#endif

        // Initialize the session
        frontEndWindow.updateCaseInfo();

        // Wait until the initialization background tasks are finished before
        // allowing data to be used in the main UI.
        while( init.isRunning() )
        {
            QThread::yieldCurrentThread();
        }

        // if the system is running low on space, turn off all storage except for the logs
//lcv        frontEndWindow.disableStorage( options.count( "low-space" ) );

//#if QT_NO_DEBUG
//        // The laser diode is turned on at the start of the case and remains on throughout
//        laser.turnDiodeOn();
//#endif

        // Start the daq and data consumer threads  // XXX needed here?  device select will start the HW
        frontEndWindow.startDaq();
        frontEndWindow.startDataCapture();

//#if QT_NO_DEBUG
//        frontEndWindow.setupDeviceForSledSupport();
//#endif

        status = app.exec();

        // Shutdown the data consumer thread and the hardware
        frontEndWindow.stopDataCapture();
        frontEndWindow.stopDaq(); // merge into stopDataCapture()?

//#if QT_NO_DEBUG
//        laser.turnDiodeOff();

//        pdb.powerOff();
//#endif

        // Set the flag indicating all has been closed properly for this session
        sessionDatabase &db = sessionDatabase::Instance();
        db.markExitAsClean();

        LOG( INFO, "Application stopped: OCT Console" )
    }
    else  // the case wizard was cancelled
    {
        // tell frontend the application isn't starting up
        frontEndWindow.abortStartUp();

//#if QT_NO_DEBUG
//        // power down
//        pdb.powerOff();
//#endif

        LOG( INFO, "Application cancelled: OCT Console" )

        // user cancelled setup; return normal exit code
        status = 0;
    }

    return status;
}
