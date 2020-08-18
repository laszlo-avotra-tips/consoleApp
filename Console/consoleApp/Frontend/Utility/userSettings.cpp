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
#include "signalmodel.h"
#include "Frontend/Widgets/DisplayOptionsModel.h"

userSettings* userSettings::theSettings{nullptr};
caseInfo* caseInfo::theInfo{nullptr};

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
    auto* displayOptionsModel = DisplayOptionsModel::instance();

    reticleBrightnessVal = displayOptionsModel->reticleBrightness();
    brightnessVal = displayOptionsModel->imageBrightness();
    contrastVal = displayOptionsModel->imageContrast();

    settings->setValue( "image/brightness",               brightnessVal );
    settings->setValue( "image/contrast",                 contrastVal );
    settings->setValue( "image/reticleBrightness",        reticleBrightnessVal );
    settings->setValue( "image/laserIndicatorBrightness", laserIndicatorBrightnessVal );
    settings->setValue( "image/noiseReduction",           noiseReductionVal );
    settings->setValue( "image/useInvertOctColor",        invertOctColorEnabled );
}

int userSettings::getImageIndexDecimation() const
{
    return imageIndexDecimation;
}


/*
 * loadSettings()
 *
 * Load the current user settings from the .ini file.
 */
userSettings &userSettings::Instance() {

    if(!theSettings){
        theSettings = new userSettings();
    }
    return *theSettings;
}

void userSettings::loadSettings()
{
    // lag angle is NOT saved across sessions

    brightnessVal               = settings->value( "image/brightness",               BrightnessLevels_HighSpeed.defaultValue ).toInt();
    contrastVal                 = settings->value( "image/contrast",                 ContrastLevels_HighSpeed.defaultValue ).toInt();
    reticleBrightnessVal        = settings->value( "image/reticleBrightness",        DefaultReticleBrightness ).toInt();
    laserIndicatorBrightnessVal = settings->value( "image/laserIndicatorBrightness", DefaultLaserIndicatorBrightness ).toInt();
    noiseReductionVal           = settings->value( "image/noiseReduction",           DefaultCurrFrameWeight_Percent ).toInt();
    invertOctColorEnabled       = settings->value( "image/useInvertOctColor",        DefaultUseInvertOctColor ).toBool();
    imageIndexDecimation        = settings->value( "log/imageIndexDecimation",       ImageIndexDecimationLog.defaultValue ).toInt();

    auto* displayOptionsModel = DisplayOptionsModel::instance();

    displayOptionsModel->setReticleBrightness(reticleBrightness());
    displayOptionsModel->setImageBrightness(brightness());
    displayOptionsModel->setImageContrast(contrast());
}

void userSettings::setBrightness(int level)
{
    brightnessVal = level;
    saveSettings();
}

void userSettings::setContrast(int level)
{
    contrastVal = level;
    saveSettings();
}

void userSettings::setCatheterView(userSettings::CatheterView_t view)
{
    catheterViewMode = view;
    SignalModel::instance()->setIsDistalToProximalView(isDistalToProximalView());
}

int userSettings::brightness()
{
    return brightnessVal;
}

int userSettings::contrast()
{
    return contrastVal;
}

bool userSettings::isDistalToProximalView()
{
    if( catheterViewMode == DistalToProximal )
    {
        return true;
    }
    else
    {
        return false;
    }
}


caseInfo &caseInfo::Instance() {

    if(!theInfo){
        theInfo = new caseInfo();
    }
    return *theInfo;
}
