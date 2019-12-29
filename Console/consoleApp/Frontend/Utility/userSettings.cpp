/*
 * userSettings.cpp
 *
 * User modifiable settings object. Contains user modifiable view option
 * settings for the OCT console software. Saved to disk so settings
 * can persist between sessions.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */

#include "userSettings.h"
#include "defaults.h"

userSettings::userSettings()
{
    settings = new QSettings( SystemSettingsFile, QSettings::IniFormat );

    loadSettings();
}

/*
 * saveSettings()
 *
 * Write the current user settings to the .ini file.
 */
void userSettings::saveSettings()
{
    settings->setValue( "image/brightness",               brightnessVal );
    settings->setValue( "image/contrast",                 contrastVal );
    settings->setValue( "image/waterfallRateVal",         waterfallRateVal );
    settings->setValue( "image/showWaterfall",            showWaterfall );
    settings->setValue( "image/reticleBrightness",        reticleBrightnessVal );
    settings->setValue( "image/laserIndicatorBrightness", laserIndicatorBrightnessVal );
    settings->setValue( "image/noiseReduction",           noiseReductionVal );
    settings->setValue( "image/useInvertOctColor",        invertOctColorEnabled );
}

/*
 * loadSettings()
 *
 * Load the current user settings from the .ini file.
 */
void userSettings::loadSettings()
{
    // lag angle is NOT saved across sessions

    brightnessVal               = settings->value( "image/brightness",               BrightnessLevels_HighSpeed.defaultValue ).toInt();
    contrastVal                 = settings->value( "image/contrast",                 ContrastLevels_HighSpeed.defaultValue ).toInt();
    waterfallRateVal            = settings->value( "image/waterfallRateVal",         DefaultWaterfallRate ).toInt();
    showWaterfall               = settings->value( "image/showWaterfall",            DefaultShowWaterfall ).toBool();
    reticleBrightnessVal        = settings->value( "image/reticleBrightness",        DefaultReticleBrightness ).toInt();
    laserIndicatorBrightnessVal = settings->value( "image/laserIndicatorBrightness", DefaultLaserIndicatorBrightness ).toInt();
    noiseReductionVal           = settings->value( "image/noiseReduction",           DefaultCurrFrameWeight_Percent ).toInt();
    invertOctColorEnabled       = settings->value( "image/useInvertOctColor",        DefaultUseInvertOctColor ).toBool();
}
