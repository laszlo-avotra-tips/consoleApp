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
#include "logger.h"

userSettings* userSettings::theSettings{nullptr};
caseInfo* caseInfo::theInfo{nullptr};

userSettings::userSettings()
{
    settings = new QSettings( SystemVarFile, QSettings::IniFormat );

    loadSettings();
}

/*
 * saveSettings()
 *
 * Write the current user settings to the .ini file.
 */
void userSettings::saveSettings()
{
    settings->setValue( "displayOptions/brightness",               brightnessVal );
    settings->setValue( "displayOptions/contrast",                 contrastVal );
    settings->setValue( "displayOptions/reticleBrightness",        reticleBrightnessVal );

    LOG3(brightnessVal,contrastVal,reticleBrightnessVal)
}

QStringList userSettings::getLocations() const
{
    return m_locations;
}

void userSettings::setLocations(const QStringList &locations)
{
    m_locations = locations;
}

QStringList userSettings::getDoctors() const
{
    return m_doctors;
}

void userSettings::setDoctors(const QStringList &doctors)
{
    m_doctors = doctors;
}

QDate userSettings::getServiceDate() const
{
    return m_serviceDate;
}

int userSettings::getImageDepthIndex() const
{
    return m_imageDepthIndex;
}

void userSettings::setImageDepthIndex(int imageDepthIndex)
{
    m_imageDepthIndex = imageDepthIndex;
}

bool userSettings::getIsGray() const
{
    return m_isGray;
}

void userSettings::setIsGray(bool isGray)
{
    m_isGray = isGray;
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
    brightnessVal               = settings->value( "displayOptions/brightness",               BrightnessLevels_HighSpeed.defaultValue ).toInt();
    contrastVal                 = settings->value( "displayOptions/contrast",                 ContrastLevels_HighSpeed.defaultValue ).toInt();
    reticleBrightnessVal        = settings->value( "displayOptions/reticleBrightness",        DefaultReticleBrightness ).toInt();
    LOG3(brightnessVal,contrastVal,reticleBrightnessVal)

    QString date               = settings->value( "service/last_service_date",        "" ).toString();
    m_serviceDate = QDate::fromString(date, "MM.dd.yyyy");
    LOG2(date,m_serviceDate.toString())

    m_doctors = settings->value( "caseSetup/doctors",        "" ).toStringList();
    for(const auto& doctor : m_doctors){
        LOG1(doctor)
    }

    m_locations = settings->value( "caseSetup/locations",        "" ).toStringList();
    for(const auto& location : m_locations){
        LOG1(location)
    }

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
