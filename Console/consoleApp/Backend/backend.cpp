#include "backend.h"
#include "logger.h"
#include "util.h"
#include <QDate>


Backend::Backend(int appId, int argc,char** argv, QObject *parent) : QObject(parent)
{
    // use the touch keyboard
//    keyboardInputContext *ic = new keyboardInputContext();
//    app.setInputContext( ic );

//    if( app.isRunning() )
//    {
//        return 0;
//    }

    // single application initialization
//    app.initialize();

    // Set-up the logging system and make sure it is OK to run
    Logger &log = Logger::Instance();
    if( !log.init( "consoleApp" ) )
    {
        displayFailureMessage( log.getStatusMessage(), true );
    }

    // Start the session in the system log
    LOG( INFO, "-------------------" )
    LOG( INFO, "Application started: OCT HS Console" )
    LOG( INFO, QString( "OCT Console Process ID (PID) : %1" ).arg( appId ) )
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

    //#if QT_NO_DEBUG
    //    // Provide power to all other non-PC components in the Lightbox
    //    powerDistributionBoard pdb;
    //    pdb.powerOn();
    //#endif

    //    // if both monitors are not present, only show the technician's
    //    if( !init.isPhysicianScreenAvailable() )
    //    {
    //        frontEndWindow.turnOffPhysicianScreen();
    //    }

    //    app.setActivationWindow( &frontEndWindow );

    //    init.setExeCheck( runExeCheck );
    m_init.init(argc,argv);

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

}

bool Backend::isPhysicianScreenAvailable()
{
    return m_init.isPhysicianScreenAvailable();
}
