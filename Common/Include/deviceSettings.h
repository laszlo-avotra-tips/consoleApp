/*
 * deviceSettings.h
 *
 * Device specific settings object. Contains physical and 
 * imaging characteristics for a particular model of imaging
 * catheter.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2017 Avinger, Inc
 */
#pragma once

#include <QString>
#include <QImage>
#include <QSettings>
#include <QDomDocument>
#include <QDebug>
#include "defaults.h"

/*
 * Device class
 */
class device
{
public:
    enum DeviceType
    {
        LowSpeed,
        HighSpeed,
        Ocelaris
    };

    /*
     * Default values for a new device
     */
    device( QString     inDeviceName,
            int         inInternalImagingMask_px   = 0,
            int         inCatheterRadius_um        = 0,
            int         inLinesPerRevolution_cnt   = 1024,  // magic
            int         inRevolutionsPerMin        = 500,   // magic
            int         inALineLengthNormal_px     = 512,   // magic
            int         inALineLengthDeep_px       = 1024,  // magic
            float       inImagingDepthNormal_mm    = 3.3,   // magic
            float       inImagingDepthDeep_mm      = 6.0,   // magic
            int         inClockingEnabled          = 1,
            QByteArray  inClockingGain             = "25",  // magic
            QByteArray  inClockingOffset           = "400", // magic
            QByteArray  inTorqueLimit              = "45",
            QByteArray  inTimeLimit                = "1",
            int         inLimitBlinkEnabled        = -1,
//            QByteArray  inReverseAngle             = "0",
            int         inMeasurementVersion       = 0,
            QByteArray  inSpeed1                   = "1000",
            QByteArray  inSpeed2                   = "1500",
            QByteArray  inSpeed3                   = "2000",
            QString     inDisclaimerText           = InvestigationalDeviceWarning,
            DeviceType  inDeviceType               = LowSpeed,
            QImage     *inIcon                     = NULL )
    {
        deviceName               = inDeviceName;
        internalImagingMask_px   = inInternalImagingMask_px;
        catheterRadius_um        = inCatheterRadius_um;
        linesPerRevolution_cnt   = inLinesPerRevolution_cnt;
        revolutionsPerMin        = inRevolutionsPerMin;
        aLineLengthNormal_px     = inALineLengthNormal_px;
        aLineLengthDeep_px       = inALineLengthDeep_px;
        imagingDepthNormal_mm    = inImagingDepthNormal_mm;
        imagingDepthDeep_mm      = inImagingDepthDeep_mm;
        clockingEnabled          = inClockingEnabled;
        clockingGain             = inClockingGain;
        clockingOffset           = inClockingOffset;
        torqueLimit              = inTorqueLimit;
        timeLimit                = inTimeLimit;
        limitBlinkEnabled        = inLimitBlinkEnabled;
//        reverseAngle             = inReverseAngle;

        disclaimerText           = inDisclaimerText;
        measurementVersion       = inMeasurementVersion;
        deviceType               = inDeviceType;
        icon                     = inIcon;
        pixelsPerMm              = (float)aLineLengthNormal_px / (float)imagingDepthNormal_mm;
        pixelsPerUm              = pixelsPerMm / (float)1000;
    }

    ~device()
    {
        if( icon != NULL )
        {
            delete icon;
        }
    }

    QString getDeviceName(void);
    int getInternalImagingMask_px(void)   { return internalImagingMask_px; }
    int getCatheterRadius_px(void)        { return ( catheterRadius_um * pixelsPerUm ); }
    int getCatheterRadius_um(void)        { return catheterRadius_um; }
    int getLinesPerRevolution(void)       { return linesPerRevolution_cnt; }
    int getRevolutionsPerMin(void)        { return revolutionsPerMin; }
    int getALineLengthNormal_px(void)     { return aLineLengthNormal_px; }
    int getALineLengthDeep_px(void)       { return aLineLengthDeep_px; }
    int getMeaurementVersion(void)        { return measurementVersion; }
    int getClockingEnabled(void)          { return clockingEnabled; }
    QByteArray getClockingGain(void)      { return clockingGain; }
    QByteArray getClockingOffset(void)    { return clockingOffset; }
    QByteArray getTorqueLimit(void)       { return torqueLimit; }
    QByteArray getTimeLimit(void)         { return timeLimit; }
    int getLimitBlinkEnabled(void)        { return limitBlinkEnabled; }
//    QByteArray getReverseAngle(void)      { return reverseAngle; }
    float getImagingDepthNormal_mm(void)  { return imagingDepthNormal_mm; }
    float getImagingDepthDeep_mm(void)    { return imagingDepthDeep_mm; }
    QImage getIcon(void)                  { return icon->copy(); }
    DeviceType getDeviceType(void)        { return deviceType; }
    bool isHighSpeed(void)                { return ((deviceType == HighSpeed) || (deviceType == Ocelaris)); }
    bool isOcelaris(void)                 { return (deviceType == Ocelaris); }
    QByteArray getSpeed1(void)            { return Speed1; }
    QByteArray getSpeed2(void)            { return Speed2; }
    QByteArray getSpeed3(void)            { return Speed3; }
    QString getDisclaimerText(void)       { return disclaimerText; }
    void setInternalImagingMask_px(int mask)  { internalImagingMask_px = mask; }

private:
    QString    deviceName;
    QImage    *icon;

    int        internalImagingMask_px;
    int        catheterRadius_um;
    int        revolutionsPerMin;
    int        linesPerRevolution_cnt;
    int        aLineLengthNormal_px;
    int        aLineLengthDeep_px;
    int        measurementVersion;
    int        clockingEnabled;
    QByteArray clockingGain;
    QByteArray clockingOffset;
    QByteArray torqueLimit;
    QByteArray timeLimit;
    int        limitBlinkEnabled;
//    QByteArray reverseAngle;
    float      imagingDepthNormal_mm;
    float      imagingDepthDeep_mm;
    float      pixelsPerMm;
    float      pixelsPerUm;
    QByteArray Speed1;
    QByteArray Speed2;
    QByteArray Speed3;
    QString    disclaimerText;
    DeviceType deviceType;
};

/*
 * deviceSettings Class
 */
class deviceSettings : public QObject
{
    Q_OBJECT

public:
    // singleton
    static deviceSettings & Instance()
    {
        static deviceSettings theSettings;
        return theSettings;
    }

    int init( void );

    void setCurrentDevice( int devIndex );

    int getCurrentDevice( void ) { return currentDevice; };
    device *deviceAt( int devIndex ) { return( deviceList.at( devIndex ) ); }

    device *current(void)
    {
        if ( currentDevice >= 0 )
        {
            return deviceList.at( currentDevice );
        }
        else
        {
            return NULL;
        }
    }

    QString getCurrentDeviceName( void )
    {
        if ( currentDevice >= 0 )
        {
            return deviceList.at( currentDevice )->getDeviceName();
        }
        else
        {
            return "Unknown";
        }
    }

    QString getFileSystemSafeDeviceName( void );

    // Only way to easily iterate the list is to pass it off
    QList<device *>list( void ) { return deviceList; }

    bool loadDevice( QString deviceFile );
    bool checkVersion( QDomDocument *doc );

    void setBrightness (int value ) { brightness = value; }
    void setContrast( int value ) { contrast = value; }
    int  getBrightness (void ) { return brightness; }
    int  getContrast( void ) { return contrast; }

signals:
    void deviceChanged( int );
    void sendWarning( QString );
    void sendFailure( QString ); // signal to consoleApp frontend
    void displayMask( int );

public slots:
    void adjustMaskSize( int );

private:
    int currentDevice;
    QList<device *> deviceList;
    QSettings *settings;

    int brightness = 0;
    int contrast = 0;

    deviceSettings();
    ~deviceSettings();

    deviceSettings(deviceSettings const &); // hide copy
    deviceSettings & operator=(deviceSettings const &); // hide assign
};
