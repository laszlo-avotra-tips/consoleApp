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
#include "deviceSettings.h"
#include "backend.h"
#include "formnavigator.h"
//#include "keyboardInputContext.h"


/*
 * main
 */
int main(int argc, char *argv[])
{
    QApplication app( argc, argv );

//    keyboardInputContext *ic = new keyboardInputContext();
//    app.setInputContext( ic );


    FormNavigator navigator;
    navigator.display();

    app.exec();

    return 0;

    Backend backEndLogic(&navigator);

    frontend frontEndWindow(&navigator);

//#if QT_NO_DEBUG
//    // Provide power to all other non-PC components in the Lightbox
//    powerDistributionBoard pdb;
//    pdb.powerOn();
//#endif

    // if both monitors are not present, only show the technician's
    if( !backEndLogic.isPhysicianScreenAvailable() )
    {
        frontEndWindow.turnOffPhysicianScreen();
    }

//    app.setActivationWindow( &frontEndWindow );

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

        QObject::connect( &app, SIGNAL( aboutToQuit() ), &frontEndWindow, SLOT( shutdownCleanup() ) );

        if(!idaq){
            frontEndWindow.abortStartUp();

            LOG( INFO, "Device not supported. OCT Console cancelled" )

            // user cancelled setup; return normal exit code
            status = 0;
            return status;
        }
        frontEndWindow.setIDAQ(idaq);

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
//
//        QObject::connect( &frontEndWindow, SIGNAL( checkSledStatus() ), &sledSupport, SLOT( getAllStatus() ) );
//
//#else // !QT_NO_DEBUG
        LOG( INFO, "SLED support board: serial port control is DISABLED" )
//#endif

        // Initialize the session
        frontEndWindow.updateCaseInfo();

//        // Wait until the initialization background tasks are finished before
//        // allowing data to be used in the main UI.
//        while( init.isRunning() )
//        {
//            QThread::yieldCurrentThread();
//        }

        // if the system is running low on space, turn off all storage except for the logs
//lcv        frontEndWindow.disableStorage( options.count( "low-space" ) );

//#if QT_NO_DEBUG
//        // The laser diode is turned on at the start of the case and remains on throughout
//        laser.turnDiodeOn();
//#endif

        // Start the daq and data consumer threads  // XXX needed here?  device select will start the HW
        frontEndWindow.startDaq();
        auto& setting = deviceSettings::Instance();
        if(setting.getIsSimulation()){
            frontEndWindow.startDataCapture();
        }
        frontEndWindow.on_zoomSlider_valueChanged(100);
//#if QT_NO_DEBUG
//        frontEndWindow.setupDeviceForSledSupport();
//#endif

        status = app.exec();

        // Shutdown the data consumer thread and the hardware
        if(setting.getIsSimulation()){
            frontEndWindow.stopDataCapture();
        }
        frontEndWindow.stopDaq(); // merge into stopDataCapture()?

//#if QT_NO_DEBUG
//        laser.turnDiodeOff();

//        pdb.powerOff();
//#endif

        // Set the flag indicating all has been closed properly for this session
        sessionDatabase db ; //lcv = sessionDatabase::Instance();
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
