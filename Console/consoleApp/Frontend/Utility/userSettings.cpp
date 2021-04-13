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

QString userSettings::getSerialNumber() const
{
    return m_serialNumber;
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

//    LOG3(brightnessVal,contrastVal,reticleBrightnessVal)

    varSettings->setValue( "displayOptions/depthIndex", m_imageDepthIndex );

    if(m_isGray){
        varSettings->setValue( "displayOptions/color", QString("gray" ));
    } else {
        varSettings->setValue( "displayOptions/color", QString("sepia") );
    }
//    LOG2(m_isGray,m_imageDepthIndex)
    varSettings->setValue( "caseSetup/physician", m_physician );
    varSettings->setValue( "caseSetup/location", m_location );
}

void userSettings::loadVarSettings()
{
    brightnessVal        = varSettings->value( "displayOptions/brightness",        0 ).toInt();
    contrastVal          = varSettings->value( "displayOptions/contrast",          0 ).toInt();
    reticleBrightnessVal = varSettings->value( "displayOptions/reticleBrightness", 127 ).toInt();
    m_imageDepthIndex = varSettings->value( "displayOptions/depthIndex", 0 ).toInt();

    QString color =  varSettings->value( "displayOptions/color", "" ).toString();
    if(color == "gray"){
        m_isGray = true;
    } else {
        m_isGray = false;
    }

    LOG3(brightnessVal,contrastVal,reticleBrightnessVal)

    QString date               = varSettings->value( "service/last_service_date",        "" ).toString();
    m_serviceDate = QDate::fromString(date, "MM.dd.yyyy");
    LOG2(date,m_serviceDate.toString())

    m_physician = varSettings->value( "caseSetup/physician",        "" ).toString();
    LOG1(m_physician)

    m_location = varSettings->value( "caseSetup/location",        "" ).toString();
}

void userSettings::loadProfileSettings()
{
    imageIndexDecimation = profileSettings->value( "log/imageIndexDecimation", 1).toInt();
    LOG1(imageIndexDecimation);

    daqIndexDecimation = profileSettings->value( "log/daqIndexDecimation", 0).toInt();
    LOG1(daqIndexDecimation);

    disableRendering = profileSettings->value( "control/disableRendering", 0).toInt();
    LOG1(disableRendering);

    disableExternalMonitor = profileSettings->value( "control/disableExternalMonitor", 0).toInt();
    LOG1(disableExternalMonitor);

    isSimulation = profileSettings->value( "control/isSimulation", 0).toInt();
    LOG1(isSimulation);

    isRecording = profileSettings->value( "control/isRecording", 0).toInt();
    LOG1(isRecording);

    isSequencial = profileSettings->value( "control/isSequencial", 0).toInt();
    LOG1(isSequencial);

    sequenceLimitL = profileSettings->value( "control/sequenceLimitL", 1).toInt();
    LOG1(sequenceLimitL);

    sequenceLimitH = profileSettings->value( "control/sequenceLimitH", 2).toInt();
    LOG1(sequenceLimitH);

    numberOfDaqBuffers = profileSettings->value( "control/numberOfDaqBuffers", 2).toInt();
    LOG1(numberOfDaqBuffers);

    measurementPrecision = profileSettings->value( "control/measurementPrecision", 2).toInt();
    LOG1(measurementPrecision);

    simDir = profileSettings->value( "control/simDir", "sim11").toString();
    LOG1(simDir);

    recordingDurationMin = profileSettings->value( "recording/durationMinimum_ms", 3000).toInt();
    LOG1(recordingDurationMin)

    m_imagingDepth_mm =  profileSettings->value( "octLaser/imagingDepth_mm", 0.0f).toFloat();
    m_aLineLength_px =  profileSettings->value( "octLaser/aLineLength_px", 0).toInt();
    LOG2(getImagingDepth_mm(), getALineLength_px())

    m_sled_firmware_version = profileSettings->value( "subSystemVersion/sled_firmware_version", "").toString();
    m_interface_firmware_version = profileSettings->value( "subSystemVersion/interface_firmware_version", "").toString();
    m_oct_firmware_version = profileSettings->value( "subSystemVersion/oct_firmware_version", "").toString();
    m_interface_hw_version = profileSettings->value( "subSystemVersion/interface_hw_version", "").toString();
    LOG4(getSled_firmware_version(), getInterface_firmware_version(), getOct_firmware_version(), getInterface_hw_version())

    m_serialNumber = profileSettings->value( "System/serialNumber", "0000").toString();
    LOG1(m_serialNumber)
}

QString userSettings::getSimDir() const
{
    return simDir;
}

int userSettings::getMeasurementPrecision() const
{
    return measurementPrecision;
}

int userSettings::getNumberOfDaqBuffers() const
{
    return numberOfDaqBuffers;
}

int userSettings::getSequenceLimitL() const
{
    return sequenceLimitL;
}

int userSettings::getSequenceLimitH() const
{
    return sequenceLimitH;
}

int userSettings::getIsSequencial() const
{
    return isSequencial;
}

int userSettings::getIsRecording() const
{
    return isRecording;
}

int userSettings::getIsSimulation() const
{
    return isSimulation;
}

void userSettings::setLocation(const QString &location)
{
    m_location = location;
    saveSettings();
}

void userSettings::setPhysician(const QString &physician)
{
    m_physician = physician;
    saveSettings();
}

QString userSettings::getLocation() const
{
    return m_location;
}

QString userSettings::getPhysician() const
{
    return m_physician;
}

int userSettings::getDisableExternalMonitor() const
{
    return disableExternalMonitor;
}

int userSettings::getDisableRendering() const
{
    return disableRendering;
}

void userSettings::setDisableRendering(int value)
{
    disableRendering = value;
}

int userSettings::getDaqIndexDecimation() const
{
    return daqIndexDecimation;
}

int userSettings::getRecordingDurationMin() const
{
    return recordingDurationMin;
}

void userSettings::setRecordingDurationMin(int value)
{
    recordingDurationMin = value;
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
    saveSettings();
}

bool userSettings::getIsGray() const
{
    return m_isGray;
}

void userSettings::setIsGray(bool isGray)
{
    m_isGray = isGray;
    saveSettings();
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
