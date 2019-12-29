/*
 * powerdistributionboard.cpp
 *
 * The power distribution board supplies power to the entire Lightbox, except
 * the PC ( The Laser, Fans, TIA, etc. ). This is powered on by driving the
 * digital voltage output channel high. This is powered off by driving the
 * digital voltage output channel low.
 *
 * See digital output gain and channel definitions in ioController.cpp
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 *
 */

#include "powerdistributionboard.h"
#include "ioController.h"
#include "util.h"
#include "logger.h"

const long PdbDigitalVoltageLow  = 0;
const long PdbDigitalVoltageHigh = 1;
const int  PdbSleepTime_ms       = 100;

/*
 * powerOn
 *
 * Drive the digital voltage output high and sleep.
 */
void powerDistributionBoard::powerOn( void )
{
#ifdef QT_NO_DEBUG
    LOG( INFO, "Power Distribution Board Power ON requested." );

    ioController &ioc = ioController::Instance();
    ioc.configureModule( &ioc.digitalOutModule ); // need to configureModule
//lcv 1504
    // Power on the Power Distribution Board.
    if( !ioc.setDigitalVoltageOut( PdbDigitalVoltageHigh ) )
    {
        // Power Distribution Board will not be powered on
        displayFailureMessage( "IO Controller failed to power on the Power Distribution Board.", true );
    }
    else
    {
        // Requires a sleep of more than 50 ms before laser.turnDiodeOn() can be called.
        Sleep( PdbSleepTime_ms );
    }
#endif
}

/*
 * powerOff
 *
 * Drive the digital voltage low to turn off the Power Distribution Board.
 */
void powerDistributionBoard::powerOff( void )
{
#ifdef QT_NO_DEBUG
    LOG( INFO, "Power Distribution Board Power OFF requested." );

    ioController &ioc = ioController::Instance();
    if( !ioc.setDigitalVoltageOut( PdbDigitalVoltageLow ) )
    {
        displayFailureMessage( "IO Controller failed to power off the Power Distribution Board.", false );
    }
#endif
}
