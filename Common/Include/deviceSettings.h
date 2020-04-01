/*
 * deviceSettings.h
 *
 * Device specific settings object. Contains physical and 
 * imaging characteristics for a particular model of imaging
 * catheter.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#pragma once

#include <QString>
#include <QImage>
#include <QSettings>
#include <QDomDocument>
#include "defaults.h"
#include <QDebug>

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
            float       inImagingDepthNormal_mm    = 3.3f,   // magic
            float       inImagingDepthDeep_mm      = 6.0f,   // magic
            bool        inClockingEnabledByDefault = false,
            QByteArray  inClockingGain             = "25",  // magic
            QByteArray  inClockingOffset           = "400", // magic
            QByteArray  inTorqueLimit              = "4.5",
            int         inTimeLimit                = 1,
            int         inLimitBlinkEnabled        = -1,
            int         inMeasurementVersion       = 0,
            QByteArray  inSpeed1                   = "1000",
            QByteArray  inSpeed2                   = "1500",
            QByteArray  inSpeed3                   = "2000",
            QString     inDisclaimerText           = InvestigationalDeviceWarning,
            DeviceType  inDeviceType               = LowSpeed,
            QImage     *inIcon                     = nullptr )
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
        clockingEnabledByDefault = inClockingEnabledByDefault;
        clockingGain             = inClockingGain;
        clockingOffset           = inClockingOffset;
        torqueLimit              = inTorqueLimit;
        timeLimit                = inTimeLimit;
        limitBlinkEnabled        = inLimitBlinkEnabled;
        Speed1                   = inSpeed1;
        Speed2                   = inSpeed2;
        Speed3                   = inSpeed3;













        disclaimerText           = inDisclaimerText;
        measurementVersion       = inMeasurementVersion;
        deviceType               = inDeviceType;
        icon                     = inIcon;
        pixelsPerMm              = static_cast<float>(aLineLengthNormal_px) / static_cast<float>(imagingDepthNormal_mm);
        pixelsPerUm              = pixelsPerMm / 1000.0f;
        rotation                 = -1;
    }

    ~device()
    {
        if( icon )
        {
            delete icon;
        }
    }

    QString getDeviceName(void)           const { return deviceName; }
    int getInternalImagingMask_px(void)   const { return internalImagingMask_px; }
    int getCatheterRadius_px(void)        const { return int( catheterRadius_um * pixelsPerUm ); }
    int getCatheterRadius_um(void)        const { return catheterRadius_um; }
    int getLinesPerRevolution(void)       const { return linesPerRevolution_cnt; }
    int getRevolutionsPerMin(void)        const { return revolutionsPerMin; }
    int getALineLengthNormal_px(void)     const { return aLineLengthNormal_px; }
    int getALineLengthDeep_px(void)       const { return aLineLengthDeep_px; }
    int getMeaurementVersion(void)        const { return measurementVersion; }
    bool isClockingEnabledByDefault(void) const { return clockingEnabledByDefault; }
    QByteArray getClockingGain(void)      const { return clockingGain; }
    QByteArray getClockingOffset(void)    const { return clockingOffset; }
    QByteArray getTorqueLimit(void)       const { return torqueLimit; }
    int getTimeLimit(void)                const { return timeLimit; }
    int getLimitBlink(void)               const { return limitBlinkEnabled; }
    float getImagingDepthNormal_mm(void)  const { return imagingDepthNormal_mm; }
    float getImagingDepthDeep_mm(void)    const { return imagingDepthDeep_mm; }
    QImage getIcon(void)                  const { return icon->copy(); }
    DeviceType getDeviceType(void)        const { return deviceType; }
    bool isHighSpeed(void)                const { return ((deviceType == HighSpeed) || (deviceType == Ocelaris)); }
    bool isOcelaris(void)                 const { return (deviceType == Ocelaris); }
    QByteArray getSpeed1(void)            const { return Speed1; }
    QByteArray getSpeed2(void)            const { return Speed2; }
    QByteArray getSpeed3(void)            const { return Speed3; }
    bool isBidirectional(void)            const { return (deviceType == Ocelaris); }
    QString getDisclaimerText(void)       const { return disclaimerText; }
    void setLinesPerRevolution(int lines) { linesPerRevolution_cnt = lines; }
    void setRevolutionsPerMin(int speed)  { revolutionsPerMin = speed; }
    void setRotation( int dir )           { rotation = dir; }
    int getRotation(void)                 const { return rotation; }

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
    bool       clockingEnabledByDefault;
    QByteArray clockingGain;
    QByteArray clockingOffset;
    QByteArray torqueLimit;
    int        timeLimit;
    int        limitBlinkEnabled;
    float      imagingDepthNormal_mm;
    float      imagingDepthDeep_mm;
    float      pixelsPerMm;
    float      pixelsPerUm;
    QByteArray Speed1;
    QByteArray Speed2;
    QByteArray Speed3;
    QString    disclaimerText;
    DeviceType deviceType;
    int        rotation;
};

/*
 * deviceSettings Class
 */
class deviceSettings : public QObject
{
    Q_OBJECT

public:
    // singleton
    static deviceSettings & Instance();
    int init( void );

    void setCurrentDevice( int devIndex )
    {
        currentDevice = devIndex;
        emit deviceChanged();
    }

    int getCurrentDevice( void ) { return currentDevice; }
    device *deviceAt( int devIndex ) { return( deviceList.at( devIndex ) ); }

    device *current(void)
    {
        if ( currentDevice >= 0 )
        {
            return deviceList.at( currentDevice );
        }
        else
        {
            return nullptr;
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
    void deviceChanged();
    void sendWarning( QString );
    void sendFailure( QString ); // signal to consoleApp frontend

private:
    int currentDevice;
    QList<device *> deviceList;
    QSettings *settings;
    static deviceSettings* theSettings;
    int brightness;
    int contrast;

    deviceSettings();
    ~deviceSettings();

    deviceSettings(deviceSettings const &); // hide copy
    deviceSettings & operator=(deviceSettings const &); // hide assign
};
