/*
 * settings.cpp
 *
 * Description:
 *
 * Author: Dennis W. Jackson
 *
 * Copyright 2009-2018 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include "daqSettings.h"
#include "defaults.h"
#include "logger.h"
#include "buildflags.h"
#include "util.h"

DaqSettings* DaqSettings::theSettings{nullptr};

// Singleton
DaqSettings & DaqSettings::Instance() {
    if(!theSettings){
        theSettings = new DaqSettings();
    }
    return *theSettings;
}

/*
 * Constructor
 */
DaqSettings::DaqSettings()
{
    isReady = false;
}

/*
 * init
 *
 * Load the DAQ hardware settings from disk.
 */
bool DaqSettings::init( void )
{
    // Only initialize the settings if they have not been before
    if( !isReady )
    {
        // Settings are saved in a hard-coded location
        settings = new QSettings( ConsoleIniFile , QSettings::IniFormat );

        if( settings )
        {
            // Keep track of where the default file is stored and the current file name
            QFileInfo iniFile( settings->fileName() );
            defaultPath = iniFile.absolutePath();
            basename    = iniFile.baseName();

            load();
            isReady = true;

            LOG( INFO, "DAQ initialization settings:" )
            LOG( INFO, QString( "   recordCount:        %1" ).arg( recordCount ) )
            LOG( INFO, QString( "   preDepth:           %1" ).arg( preDepth ) )
            LOG( INFO, QString( "   recordLength:       %1" ).arg( recordLength ) )
            LOG( INFO, QString( "   chAInputRange:      %1" ).arg( chAInputRange ) )
            LOG( INFO, QString( "   chBInputRange:      %1" ).arg( chBInputRange ) )
            // TBD - encoder min/max are values to start looking for the encoder.  they are DAQ specific
            LOG( INFO, QString( "   encoderCalMin:      %1" ).arg( encoderCalMin ) )
            LOG( INFO, QString( "   encoderCalMax:      %1" ).arg( encoderCalMax ) )
        }
        else
        {
            // isReady stays false.  Send a message to the debug system.
            // The Caller handles errors.
            qDebug() << "Could not load " << ConsoleIniFile;
        }
    }
    return isReady;
}

/*
 * Destructor
 */
DaqSettings::~DaqSettings()
{
    if( settings )
    {
        delete settings;
    }
}

/*
 * load
 *
 * Load device settings from disk.  Reasonable defaults are defined for settings
 * that are not set in the INI file.
 *
 * The laser profile is dependent preDepth and recordLength; using the default
 * values will provide an image but it may be truncated.
 *
 * The input ranges for Ch A and B are defined in DAQ.h.
 *
 * The default encoder values are within the expected half-scale 14-bit range
 * (8192 - 16383).  The DSP automatically determines the full-range of the encoder
 * during the first few turns.
 */
void DaqSettings::load( void )
{
    settings->beginGroup( "DAQ" );

    recordCount        = settings->value( "recordCount",            1 ).value<unsigned long>();
    preDepth           = settings->value( "preDepth",             416 ).value<unsigned long>();
    recordLength       = settings->value( "recordLength",        4096 ).value<unsigned long>();
    chAInputRange      = settings->value( "chAInputRange",         12 ).value<unsigned long>();
    chBInputRange      = settings->value( "chBInputRange",         14 ).value<unsigned long>();

    // XXX: use 14-bit values since we are shifting the encoder values in SoftwareDSP.  Using the full range
    //      caused integer overflow in the encoder calculations. Needs more investigating.
    encoderCalMin      = settings->value( "encoderMin",         9500 ).value<unsigned short>();
    encoderCalMax      = settings->value( "encoderMax",         12000 ).value<unsigned short>();
//    encoderCalMin      = settings->value( "encoderMin",         38000 ).value<unsigned short>();
//    encoderCalMax      = settings->value( "encoderMax",         50000 ).value<unsigned short>();

    settings->endGroup();

    /*
     * Verify that the data lengths are suitable for the hardware.  No one outside
     * of SW development should ever see this warning since OCT DAQ Config sets these
     * values and it forces alignment.
     */
    if( ( ( preDepth     % DaqRecordSampleAlignment ) != 0 ) ||
        ( ( recordLength % DaqRecordSampleAlignment ) != 0 ) )
    {
        displayWarningMessage( QString( "Pre-trigger (%1) or Record Length (%2) is not a multiple of the DAQ alignment (%3) " ).arg( preDepth ).arg( recordLength ).arg( DaqRecordSampleAlignment ) );
    }
}
