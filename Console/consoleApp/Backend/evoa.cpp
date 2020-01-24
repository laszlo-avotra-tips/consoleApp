/*
 * evoa.cpp
 *
 * Use the IO Controller to command an output voltage that is then applied to the
 * Electronic Variable Optical Attenuator (EVOA).
 *
 * See analog output gain and channel definitions in ioController.cpp
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#include <QSettings>
#include <QTextStream>
#include "evoa.h"
#include "ioController.h"
#include "logger.h"
#include "defaults.h"
#include "deviceSettings.h"

/*
 * Constructor
 */
EVOA::EVOA()
{
    scale = 1.0; // something.

    QSettings *settings = new QSettings( SystemSettingsFile, QSettings::IniFormat );
    currVoltage_v = settings->value( EvoaDefaultSetting, EvoaDefault_v ).toDouble();
    currStatus = EvoaStatusDefault;

    // initialize the channel used by the EVOA
    ioController &ioc = ioController::Instance();
    ioc.configureModule( &ioc.analogOutModule );

    // Set the voltage to the default starting value
    setVoltage( currVoltage_v, currStatus );
}

/*
 * Destructor
 */
EVOA::~EVOA()
{
    // destructor
}

/*
 * setVoltage
 *
 * Calls ioController::setAnalogVoltageOut(), and updates the current voltage variable
 * if successful.
 */
bool EVOA::setVoltage( double val, QString statusLabel )
{
    ioController &ioc = ioController::Instance();
    bool status = ioc.setAnalogVoltageOut( val );

    if( status )
    {
        currVoltage_v = val;
        currStatus = statusLabel;
        LOG( INFO, QString( "EVOA voltage(V)      = %1, status = %2" ).arg( currVoltage_v ).arg( currStatus ) )
        emit valueChanged( currVoltage_v );
        emit statusChanged( currStatus );    // set status label
    }
    else
    {
        QString str;
        QTextStream qts(&str);
        qts << "EVOA failed trying to set voltage. Attempting: "
            << val << "(V), Min: "
            << EvoaMinVoltage_v << "(V), Max: "
            << EvoaMaxVoltage_v << "(V)";
        LOG( WARNING, str )
    }

    return status;
}

/*
 * setVoltageToDefault
 */
void EVOA::setVoltageToDefault( void )
{
    QSettings *s = new QSettings( SystemSettingsFile, QSettings::IniFormat );
    setVoltage( s->value( EvoaDefaultSetting, EvoaDefault_v ).toDouble(), EvoaStatusDefault );
}

/*
 * getCurrVoltage
 */
double EVOA::getCurrVoltage( void )
{
    return currVoltage_v;
}

/*
 * updateVoltage
 *
 * Given an update on the levelGauge, change the voltage if appropriate.
 */
void EVOA::updateVoltage( double newVoltage )
{
    if( EvoaMinVoltage_v <= newVoltage && newVoltage <= EvoaMaxVoltage_v )
    {
        setVoltage( newVoltage, EvoaStatusSet );
    }
    else
    {
        QString str;
        QTextStream qts(&str);
        qts << "EVOA failed trying to set voltage outside allowed range. Attempting: "
            << newVoltage << "(V), Min: "
            << EvoaMinVoltage_v << "(V), Max: "
            << EvoaMaxVoltage_v << "(V)";

        LOG( WARNING, str )
    }
}

/*
 * calculateScaleFactor
 *
 * Calculate the mapping factor from Gauge levels to EVOA domain.
 */
void EVOA::calculateScaleFactor( int numChunks )
{
    scale = ( EvoaMaxVoltage_v - EvoaMinVoltage_v ) / double( numChunks - 1 );
}

/*
 * getMinVal
 */
float EVOA::getMinVal( void )
{
    return EvoaMinVoltage_v;
}

/*
 * getMaxVal
 */
float EVOA::getMaxVal( void )
{
    return EvoaMaxVoltage_v;
}

/*
 * pauseEvoa - store the last good voltage and set the current voltage to 0.
 */
void EVOA::pauseEvoa( bool pause )
{
    if( pause )
    {
        if( currStatus != EvoaStatusOff )
        {
            cachedEvoaStatus = currStatus;
            cachedEvoaVoltage_v = currVoltage_v;
            setVoltage( 0, EvoaStatusOff );
        }
    }
    else
    {
        if( currStatus == EvoaStatusOff )
        {
            setVoltage( cachedEvoaVoltage_v, cachedEvoaStatus );
        }
    }
}
