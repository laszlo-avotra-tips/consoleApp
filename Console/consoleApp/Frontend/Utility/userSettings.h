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
#include "defaults.h"
#include "deviceSettings.h"
#include "buildflags.h"

class caseInfo
{
public:

    // Singleton
    static caseInfo & Instance() {
        static caseInfo theInfo;
        return theInfo;
    }
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
};

/*
 * userSettings
 */
class userSettings 
{
public:
    // Singleton
    static userSettings & Instance() {
        static userSettings theSettings;
        return theSettings;
    }

    enum CatheterView_t
    {
        DistalToProximal,
        ProximalToDistal
    };

    void loadSettings();
    void setBrightness(int level)
    {
        brightnessVal = level;
        saveSettings();
    }
    void setContrast(int level)
    {
        contrastVal = level;
        saveSettings();
    }
    void setLag(int lag)
    {
        lagAngleVal = lag;
        saveSettings();
    }
    void setWaterfallRate(int rate)
    {
        waterfallRateVal = rate;
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
    void setWaterfall( bool val )
    {
        showWaterfall = val;
        saveSettings();
    }
    void setInvertOctColor( bool val )
    {
        invertOctColorEnabled = val;
        saveSettings();
    }
    void setCatheterView( CatheterView_t view ) { catheterViewMode = view; }

    int  brightness()               { return brightnessVal; }
    int  contrast()                 { return contrastVal; }
    int  lagAngle()                 { return lagAngleVal; }
    int  waterfallRate()            { return waterfallRateVal; }
    int  reticleBrightness()        { return reticleBrightnessVal; }
    int  noiseReduction()           { return noiseReductionVal; }
    int  laserIndicatorBrightness() { return laserIndicatorBrightnessVal; }
    bool useNoiseReduction()        { return noiseReductionEnabled; }
    bool waterfall()                { return showWaterfall; }
    bool invertOctColor()           { return invertOctColorEnabled; }
    bool isDistalToProximalView()
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

private:
    void saveSettings();

    int  brightnessVal;               // or "black-level"
    int  contrastVal;                 // or "white-level"
    int  lagAngleVal;                 // catheter rotational lag
    int  waterfallRateVal;            // how fast the waterfall scrolls across the screen
    int  reticleBrightnessVal;        // how bright the reticle is on the screen
    int  laserIndicatorBrightnessVal; // how bright the laser indicator pointer is on the screen
    int  noiseReductionVal;           //
    bool noiseReductionEnabled;       //
    bool showWaterfall;               //
    bool invertOctColorEnabled;       //
    QString catheterViewStr;          // view orientation of the catheter to coordinate with the fluoro view
    CatheterView_t catheterViewMode;  //

    userSettings(); // hide ctor
    ~userSettings() {} // hide dtor
    userSettings(userSettings const &); // hide copy
    userSettings & operator=(userSettings const &); // hide assign

    QSettings *settings;
};
