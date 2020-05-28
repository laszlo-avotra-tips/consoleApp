#include "backend.h"
#include "logger.h"
#include "util.h"
#include "Widgets/caseinfowizardpage.h"
#include "Widgets/caseinfowizard.h"
#include <QDate>


Backend::Backend(QWidget *parent) : QWidget(parent)
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

    //    app.setActivationWindow( &frontEndWindow );

    //    init.setExeCheck( runExeCheck );
    m_init.init();

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

void Backend::parseOptions(QCommandLineOption &options, QStringList args)
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
