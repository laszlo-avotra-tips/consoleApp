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
    varSettings = new QSettings( SystemVarFile, QSettings::IniFormat );
    profileSettings = new QSettings( SystemProfileFile, QSettings::IniFormat );

    loadSettings();
}

/*
 * saveSettings()
 *
 * Write the current user settings to the .ini file.
 */
void userSettings::saveSettings()
{
    varSettings->setValue( "displayOptions/brightness",               brightnessVal );
    varSettings->setValue( "displayOptions/contrast",                 contrastVal );
    varSettings->setValue( "displayOptions/reticleBrightness",        reticleBrightnessVal );

    LOG3(brightnessVal,contrastVal,reticleBrightnessVal)
}

void userSettings::loadVarSettings()
{
    brightnessVal               = varSettings->value( "displayOptions/brightness",               BrightnessLevels_HighSpeed.defaultValue ).toInt();
    contrastVal                 = varSettings->value( "displayOptions/contrast",                 ContrastLevels_HighSpeed.defaultValue ).toInt();
    reticleBrightnessVal        = varSettings->value( "displayOptions/reticleBrightness",        DefaultReticleBrightness ).toInt();
    LOG3(brightnessVal,contrastVal,reticleBrightnessVal)

    QString date               = varSettings->value( "service/last_service_date",        "" ).toString();
    m_serviceDate = QDate::fromString(date, "MM.dd.yyyy");
    LOG2(date,m_serviceDate.toString())

    m_doctors = varSettings->value( "caseSetup/doctors",        "" ).toStringList();
    for(const auto& doctor : m_doctors){
        LOG1(doctor)
    }

    m_locations = varSettings->value( "caseSetup/locations",        "" ).toStringList();
    for(const auto& location : m_locations){
        LOG1(location)
    }
}

void userSettings::loadProfileSettings()
{
    imageIndexDecimation = profileSettings->value( "log/imageIndexDecimation", 0).toInt();
    LOG1(imageIndexDecimation)

    m_imagingDepth_mm =  profileSettings->value( "octLaser/imagingDepth_mm", 0.0f).toFloat();
    m_aLineLength_px =  profileSettings->value( "octLaser/aLineLength_px", 0).toInt();
    LOG2(getImagingDepth_mm(), getALineLength_px())

    m_sled_firmware_version = profileSettings->value( "subSystemVersion/sled_firmware_version", "").toString();
    m_interface_firmware_version = profileSettings->value( "subSystemVersion/interface_firmware_version", "").toString();
    m_oct_firmware_version = profileSettings->value( "subSystemVersion/oct_firmware_version", "").toString();
    m_interface_hw_version = profileSettings->value( "subSystemVersion/interface_hw_version", "").toString();
    LOG4(getSled_firmware_version(), getInterface_firmware_version(), getOct_firmware_version(), getInterface_hw_version())
}

QString userSettings::getInterface_hw_version() const
{
    return m_interface_hw_version;
}

QString userSettings::getOct_firmware_version() const
{
    return m_oct_firmware_version;
}

QString userSettings::getInterface_firmware_version() const
{
    return m_interface_firmware_version;
}

QString userSettings::getSled_firmware_version() const
{
    return m_sled_firmware_version;
}

int userSettings::getALineLength_px() const
{
    return m_aLineLength_px;
}

float userSettings::getImagingDepth_mm() const
{
    return m_imagingDepth_mm;
}

void userSettings::loadSettings()
{
    loadVarSettings();
    loadProfileSettings();
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
