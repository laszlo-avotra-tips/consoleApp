/*
 * ioController.cpp
 *
 * This is the driver for the Data Translation USB GPIO module. The ioController
 * is responsible for providing power to the laser and optics system as well as
 * driving control voltages for the EVOA control loop.
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QSettings>
#include "ioController.h"
#include "logger.h"
#include "buildflags.h"
#include "defaults.h"
#include "deviceSettings.h"

ioController* ioController::theController{nullptr};

BOOL CALLBACK GetDriver( LPSTR ioControllerLpszName, LPSTR ioControllerLpszEntry, LPARAM ioControllerLParam );
//lcv
//const double       IocPositiveDaqResolution_cnts = 2048; // resolution is (1/2)*2^12 from 0-10V.
//const double       IocPositiveVoltageRange_v     = 10.0;
//const double       IocCountsPerVolt  = IocPositiveDaqResolution_cnts / IocPositiveVoltageRange_v;
//const double       IocEvoaGain_Ch0         = 1.0;
//const double       IocLaserPowerGain_Ch0   = 1.0;
//const unsigned int IocEvoaChannel          = 0;
//const unsigned int IocLaserPowerChannel    = 0;

#define STRLEN 80     // string size for general text manipulation
//char iocStr[STRLEN];  // global string for general text manipulation

typedef struct tag_board      // DAC struct for identifying device
{
    HDRVR hdrvr;              // driver handle
//    HDASS hdass;              // subsystem handle
//    ECODE status;             // board error status
    char name[STRLEN];        // string for board name
    char entry[STRLEN];       // string for board name
} BOARD;

typedef BOARD FAR* LPBOARD;

//BOARD board;

// Singleton
ioController & ioController::Instance()
{
    if(!theController)
    {
        theController = new ioController();
    }
    return *theController;
}

/*
 * Constructor
 */
ioController::ioController()
{
    analogOutModule.mode       = AnalogOutput;
//    analogOutModule.tag        = OLSS_DA;
    analogOutModule.isEnabled  = false;

    digitalOutModule.mode      = DigitalOutput;
//    digitalOutModule.tag       = OLSS_DOUT;
    digitalOutModule.isEnabled = false;

    deviceIsConfigured = false;
}

/*
 * Destructor
 */
ioController::~ioController()
{
    shutdown();
}

/*
 * GetDriver()
 *
 * This is a callback function of olDaEnumBoards, it gets the
 * strings of the Open Layers board and attempts to initialize
 * the board.  If successful, enumeration is halted.
 */
BOOL CALLBACK GetDriver( LPSTR ioControllerLpszName, LPSTR ioControllerLpszEntry, LPARAM ioControllerLParam )
{
    LOG3(ioControllerLpszName,ioControllerLpszEntry,ioControllerLParam)
//    LPBOARD lpboard = (LPBOARD)(LPVOID)ioControllerLParam;

//    /* fill in board strings */

//    lstrcpyn((PTSTR)(LPWSTR)lpboard->name,(LPCWSTR)ioControllerLpszName,MAX_BOARD_NAME_LENGTH-1);
//    lstrcpyn((PTSTR)(LPWSTR)lpboard->entry,(LPCWSTR)ioControllerLpszEntry,MAX_BOARD_NAME_LENGTH-1);

//    /* try to open board */
//    lpboard->status = olDaInitialize((PTSTR)ioControllerLpszName,(PHDEV)&lpboard->hdrvr);
//    if   (lpboard->hdrvr )
//        return false;          /* false to stop enumerating */
//    else
        return true;           /* true to continue          */
}

/*
 * queryDevice()
 *
 * Verify that the controller is present. Returns true if present.
 */
bool ioController::queryDevice( void )
{
    bool status = true;

//    mutex.lock();
//    ECODE retVal = olDaEnumBoards( (DABRDPROC)GetDriver, (LPARAM)&board );
//    mutex.unlock();

//    if( retVal )
//    {
//        LOG( FATAL, QString( "olDaEnumBoards() Failed, error code: %1" ).arg( retVal ) );
//        status = false;
//    }

    return status;
}

/*
 * configureModule()
 *
 * This initializes the board. Call this before setting a voltage on that module. Each time a
 * new module is configured, the other modules are disabled.
 *
 * Returns true if successfully enabled module, false if failed to enable
 */
bool ioController::configureModule( ioModule *module )
{
    bool  status = true;
    LOG1(module)
//    ECODE retVal = 0;
//    if( !module->isEnabled )
//    {
//        if( status )
//        {
//            mutex.lock();
//            retVal = olDaEnumBoards( (DABRDPROC)GetDriver, (LPARAM)&board ); // Get a handle to the board
//            mutex.unlock();

//            if( retVal > 0 )
//            {
//                LOG( FATAL, QString( "olDaEnumBoards() Failed, error code: %1" ).arg( retVal ) );
//                status = false;
//            }
//        }

//        // Enter only if the device has been configured
//        if( status && deviceIsConfigured )
//        {
//            mutex.lock();
//            // necessary to release previous mode
//            retVal = olDaReleaseDASS( board.hdass );
//            mutex.unlock();

//            if( retVal > 0 )
//            {
//                LOG( FATAL, QString( "olDaReleaseDASS() Failed, error code: %1" ).arg( retVal ) );
//                status = false;
//            }
//        }

//        if( status)
//        {
//            mutex.lock();
//            // Switch the ioModule tag
//            retVal = olDaGetDASS( (HDEV)board.hdrvr, module->tag, 0, &board.hdass );
//            mutex.unlock();

//            if( retVal > 0 )
//            {
//                LOG( FATAL, QString( "olDaGetDASS() Failed, error code: %1" ).arg( retVal ) );
//                status = false;
//            }
//        }

//        if( status)
//        {
//            mutex.lock();
//            retVal = olDaSetDataFlow( board.hdass, OL_DF_SINGLEVALUE );
//            mutex.unlock();

//            if( retVal )
//            {
//                LOG( FATAL, QString( "olDaSetDataFlow() Failed, error code: %1" ).arg( retVal ) );
//                status = false;
//            }
//        }

//        if( status)
//        {
//            mutex.lock();
//            retVal = olDaConfig( board.hdass );
//            mutex.unlock();

//            if( retVal )
//            {
//                LOG( FATAL, QString( "olDaConfig() Failed, error code: %1" ).arg( retVal ) );
//                status = false;
//            }
//        }
//    }

//    // Disable all modules, and enable the current module if configure was successful
//    disableAllModules();
//    if( status )
//    {
//        module->isEnabled  = true;
//        deviceIsConfigured = true;
//    }
//    else
//    {
//        emit sendError( QString( "IO Controller configureModule() has failed -- mode: %1" ).arg( module->mode ) );
//    }
    return status;
}

/*
 * disableAllModules()
 */
void ioController::disableAllModules( void )
{
    digitalOutModule.isEnabled = false;
    analogOutModule.isEnabled  = false;
}

/*
 * shutdown()
 *
 * Required to close down communication to the IO Controller.
 *
 * Returns false if any part fails.
 */
bool ioController::shutdown( void )
{
    bool  status = true;

//#ifdef QT_DEBUG
//    return status;
//#endif

//    ECODE retVal = 0;

//    // only required if a module is already configured
//    if( status && deviceIsConfigured )
//    {
//        mutex.lock();
//        retVal = olDaReleaseDASS( board.hdass );
//        mutex.unlock();
//        if( retVal )
//        {
//            LOG( FATAL, QString( "olDaReleaseDASS() Failed, error code: %1" ).arg( retVal ) );
//            status = false;
//        }
//    }

//    if( status )
//    {
//        mutex.lock();
//        retVal = olDaTerminate( (HDEV)board.hdrvr );
//        mutex.unlock();
//        if( retVal )
//        {
//            LOG( FATAL, QString( "olDaTerminate() Failed, error code: %1" ).arg( retVal ) );
//            status = false;
//        }
//    }

//    if( status )
//    {
//        disableAllModules();
//        deviceIsConfigured = false;
//    }
//    else
//    {
//        emit sendError( "IO Controller shutdown has failed." );
//    }
    return status;
}

/*
 * isReady()
 *
 * Accessor function to report the configuration status of the IO Controller.
 */
bool ioController::isReady( void )
{
    return deviceIsConfigured;
}

/*
 * setAnalogVoltageOut()
 *
 * Command the IO Controller to output the given voltage. If the EVOA isn't configured,
 * it will call configureModule() and then continue setting the voltage.
 *
 * Returns false if any part fails.
 */
bool ioController::setAnalogVoltageOut( double val )
{
    bool  status = true;
    LOG1(val)
//    ECODE retVal = 0;

//    // Convert val from volts to counts.
//    long newVal_cnts = (long)( val * IocCountsPerVolt );

//    // Verify that the board is ready
//    if( !analogOutModule.isEnabled )
//    {
//        status = configureModule( &analogOutModule );
//    }

//    if( status )
//    {
//        mutex.lock();
////        LOG4(val, newVal_cnts, IocEvoaChannel, IocEvoaGain_Ch0);
//        retVal = olDaPutSingleValue( board.hdass, newVal_cnts, IocEvoaChannel, IocEvoaGain_Ch0 );
//        mutex.unlock();

//        if( retVal )
//        {
//            LOG( FATAL, QString( "olDaPutSingleValue() Failed, error code: %1" ).arg( retVal ) );
//            status = false;
//        }
//    }

//    if( !status )
//    {
//        emit sendError( "IO Controller analog output communication has failed." );
//    }
    return status;
}

/*
 * setDigitalVoltageOut()
 *
 * Outputs a given digital voltage on the IO Controller. The Digital Voltage channel
 * is used to power on the Power Distribution Board. Logic High is around 3.3v and
 * Logic Low is 0v.
 *
 * Returns false if any part fails.
 */
bool ioController::setDigitalVoltageOut( long val )
{
    bool  status = true;
    LOG1(val)
//    ECODE retVal = 0;
//    // Verify that the board is ready
//    if( !digitalOutModule.isEnabled )
//    {
//        status = configureModule( &digitalOutModule );
//    }

//    if( status )
//    {
//        mutex.lock();
//        retVal = olDaPutSingleValue( board.hdass, val, IocLaserPowerChannel, IocLaserPowerGain_Ch0 );
//        mutex.unlock();

//        if( retVal )
//        {
//                LOG( FATAL, QString( "olDaPutSingleValue() Failed, error code: %1" ).arg( retVal ) );
//                status = false;
//        }
//    }

//    if( !status )
//    {
//        emit sendError( "IO Controller digital output communication has failed." );
//    }
    return status;
}
