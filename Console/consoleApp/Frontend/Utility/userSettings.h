/*
 * userSettings.h
 *
 * User modifiable settings object. Contains all user modifiable
 * settings for the OCT console software. Saved to disk for recovery
 * purposes, but otherwise not permanently stored.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger
 */

#pragma once

#include <QSettings>
#include <QDate>

#include "defaults.h"
#include "deviceSettings.h"

class caseInfo
{
public:

    // Singleton
    static caseInfo & Instance();
    void setPatientID( QString id ) {
        patientID = id;
    }
    void setCaseID( QString id ) {
        caseIDSet = true;
        caseID = id;
    }
    void setDoctor( QString name ) {
        doctor = name;
    }
    void setLocation( QString name ) {
        location = name;
    }
    void setDeviceName( QString name ) {
        deviceName = name;
    }
    void setNotes( QString txt ) {
        notes = txt;
    }
    void setUtcOffset( int offset ) {
        utcOffset = offset;
    }
    QString getPatientID( void ) {
        return( patientID );
    }
    QString getCaseID( void ) {
        return( caseID );
    }
    QString getDoctor( void ) {
       return( doctor );
    }
    QString getLocation( void ) {
       return( location );
    }
    QString getNotes( void ) {
       return( notes );
    }
    QString getStorageDir( void ) {
        return QString( DataDir + "/" + caseID );
    }
    bool storageValid( void ) {
        return caseIDSet;
    }
    QString getCapturesDir( void ) {
        return getStorageDir() + QString( "/captures" );
    }
    QString getClipsDir( void ) {
        return getStorageDir() + QString( "/clips" );
    }
    QString getFullCaseDir( void ) {
        return getStorageDir() + QString( "/fullCase" );
    }
    int getUtcOffset( void ) {
        return utcOffset;
    }

private:

    caseInfo() { caseIDSet = false; }
    ~caseInfo() { }
    caseInfo(caseInfo const &); // hide copy
    caseInfo & operator=(caseInfo const &); // hide assign
    bool    caseIDSet;
    QString patientID;
    QString caseID;
    QString doctor;
    QString location;
    QString deviceName;
    QString notes;
    int utcOffset;
    static caseInfo* theInfo;
};

/*
 * userSettings
 */
class userSettings 
{
public:
    // Singleton
    static userSettings & Instance();

    enum CatheterView_t
    {
        DistalToProximal,
        ProximalToDistal
    };

    void loadSettings();
    void setBrightness(int level);
    void setContrast(int level);
    void setLag(int lag)
    {
        lagAngleVal = lag;
        saveSettings();
    }
    void setReticleBrightness(int val)
    {
        reticleBrightnessVal = val;
        saveSettings();
    }
    void setLaserIndicatorBrightness(int val)
    {
        laserIndicatorBrightnessVal = val;
        saveSettings();
    }
    void setNoiseReductionVal( int val )
    {
        noiseReductionVal = val;
        saveSettings();
    }
    void setInvertOctColor( bool val )
    {
        invertOctColorEnabled = val;
        saveSettings();
    }
    void setCatheterView( CatheterView_t view );

    int  brightness();
    int  contrast();
    int  lagAngle()                 { return lagAngleVal; }
    int  reticleBrightness()        { return reticleBrightnessVal; }
    int  noiseReduction()           { return noiseReductionVal; }
    int  laserIndicatorBrightness() { return laserIndicatorBrightnessVal; }
    bool useNoiseReduction()        { return noiseReductionEnabled; }
    bool invertOctColor()           { return invertOctColorEnabled; }
    bool isDistalToProximalView();

    int getImageIndexDecimation() const;

    bool getIsGray() const;
    void setIsGray(bool isGray);

    int getImageDepthIndex() const;
    void setImageDepthIndex(int imageDepthIndex);

    QDate getServiceDate() const;

    QStringList getPhysicians() const;
    void setPhysicians(const QStringList &doctors);

    QStringList getLocations() const;
    void setLocations(const QStringList &locations);

    float getImagingDepth_mm() const;

    int getALineLength_px() const;

    QString getSled_firmware_version() const;

    QString getInterface_firmware_version() const;

    QString getOct_firmware_version() const;

    QString getInterface_hw_version() const;

private:
    void saveSettings();
    void loadVarSettings();
    void loadProfileSettings();

    int  brightnessVal;               // or "black-level"
    int  contrastVal;                 // or "white-level"
    int  lagAngleVal;                 // catheter rotational lag
    int  reticleBrightnessVal;        // how bright the reticle is on the screen
    int  laserIndicatorBrightnessVal; // how bright the laser indicator pointer is on the screen
    int  noiseReductionVal;           //
    bool noiseReductionEnabled;       //
    bool invertOctColorEnabled;       //
    int  imageIndexDecimation;        //
    QDate m_serviceDate;
    QStringList m_physicians;
    QStringList m_locations;
    QString catheterViewStr;          // view orientation of the catheter to coordinate with the fluoro view
    CatheterView_t catheterViewMode;  //
    bool m_isGray{true};
    int  m_imageDepthIndex{1};

    float m_imagingDepth_mm{0.0f};
    int m_aLineLength_px{0};
    QString m_sled_firmware_version;
    QString m_interface_firmware_version;
    QString m_oct_firmware_version;
    QString m_interface_hw_version;

    userSettings(); // hide ctor
    ~userSettings() {} // hide dtor
    userSettings(userSettings const &); // hide copy
    userSettings & operator=(userSettings const &); // hide assign

    QSettings *varSettings{nullptr};
    QSettings *profileSettings{nullptr};
    static userSettings* theSettings;
};
