/*
 * daqSettings.cpp
 *
 * Description:  stub file for backend unit tests
 *
 * Author: Dennis W. Jackson
 *
 * Copyright 2009-2010 Avinger, Inc.
 *
 */
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include "daqSettings.h"
#include "defaults.h"
#include "logger.h"

/*
 * Constructor
 */
DaqSettings::DaqSettings()
{
    load();  // make sure the stub has valid data
    isReady = false;
}

/*
 * init
 *
 * Load the DAQ hardware settings from disk.
 */
bool DaqSettings::init( void )
{
    return isReady;
};

/*
 * Destructor
 */
DaqSettings::~DaqSettings()
{

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
    encoderCalMin         = 9500;
    encoderCalMax         = 12000;
    encoderCounts         = 1024;
}
