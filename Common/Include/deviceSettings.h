﻿/*
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

#include <vector>

using DeviceIconType = std::vector<QImage*>;
/*
 * Device class
 */
class device
{
public:

/*
 * Default values for a new device
 */
  device( QString     inDeviceName,
          QByteArray  inCatheterType             = "ATH",
          QByteArray  inDevicePropVersion        = "3.0",
          QByteArray  inSledFwMinVersion         = "3.0",

          QByteArray  inIfFwMinVersion           = "3.0",
          int         inCatheterLength           = 140,
          int         inInternalImagingMask_px   = 135,
          int         inCatheterRadius_um        = 406,

          int         inBidirectional            = 0,
          int         inNumberOfSpeeds           = 1,
          int         inRevolutionsPerMin1       = 600,
          int         inRevolutionsPerMin2       = 800,

          int         inRevolutionsPerMin3       = 1000,
          int         inDefaultSpeedIndex        = 1,
          int         inClockingEnabled          = 1,
          QByteArray  inClockingGain             = "25",

          QByteArray  inClockingOffset           = "400",
          QByteArray  inTorqueLimit              = "45",
          QByteArray  inTorqueTime               = "8",
          QByteArray  inStallBlinking            = "1",

          QByteArray  inButtonMode               = "0",
          int         inMeasurementVersion       = 0,
          QString     inDisclaimerText           = InvestigationalDeviceWarning,
          QByteArray  inDeviceCRC                = "",

          DeviceIconType      inIcon = std::vector<QImage*>())
    {
        deviceName               = inDeviceName;
        splitDeviceName          = formatDeviceName(inDeviceName);
        catheterType             = inCatheterType;
        devicePropVersion        = inDevicePropVersion;
        sledFwMinVersion         = inSledFwMinVersion;
        ifFwMinVersion           = inIfFwMinVersion;
        catheterLength           = inCatheterLength;
        internalImagingMask_px   = inInternalImagingMask_px;
        catheterRadius_um        = inCatheterRadius_um;
        biDirectional            = inBidirectional;
        numberOfSpeeds           = inNumberOfSpeeds;
        revolutionsPerMin1       = inRevolutionsPerMin1;
        revolutionsPerMin2       = inRevolutionsPerMin2;
        revolutionsPerMin3       = inRevolutionsPerMin3;
        defaultSpeedIndex        = inDefaultSpeedIndex;
        aLineLength_px     = 512;
        imagingDepth_mm    = (float) 3.18;
        clockingEnabled          = inClockingEnabled;
        clockingGain             = inClockingGain;
        clockingOffset           = inClockingOffset;

        torqueLimit              = inTorqueLimit;
        torqueTime               = inTorqueTime;
        stallBlinking            = inStallBlinking;
        buttonMode               = inButtonMode;
        measurementVersion       = inMeasurementVersion;
        disclaimerText           = inDisclaimerText;
        deviceCRC                = inDeviceCRC;
        icon                     = inIcon;
        pixelsPerMm              = (float)aLineLength_px / (float)imagingDepth_mm;
        pixelsPerUm              = pixelsPerMm / (float)1000;
        m_isAth = inCatheterType[0] == 'A';
    }

    ~device()
    {
        for( auto ii : icon)
        {
            delete ii;
        }
    }

    const QString& getDeviceName(void)           { return deviceName; }
    const QString &getSplitDeviceName() const;
    QString getCatheterType(void)         { return catheterType; }
    int getInternalImagingMask_px(void) const   { return internalImagingMask_px; }
    int getCatheterRadius_px(void) const       { return ( catheterRadius_um * pixelsPerUm ); }
    int getCatheterRadius_um(void) const       { return catheterRadius_um; }
    int getRevolutionsPerMin(void) const       { return revolutionsPerMin1; }
    int getRevolutionsPerMin1(void)const       { return revolutionsPerMin1; }
    int getRevolutionsPerMin2(void)const       { return revolutionsPerMin2; }
    int getRevolutionsPerMin3(void)const       { return revolutionsPerMin3; }
    int getALineLength_px(void) const;
    int getMeaurementVersion(void)         { return measurementVersion; }
    int getClockingEnabled(void) const     { return clockingEnabled; }
    QByteArray getClockingGain(void) const { return clockingGain; }
    QByteArray getClockingOffset(void)const{ return clockingOffset; }

    QByteArray getTorqueLimit(void) const   { return torqueLimit; }
    QByteArray getTorqueTime(void)  const   { return torqueTime; }
    QByteArray getStallBlinking(void) const { return stallBlinking;}
    QByteArray getButtonMode(void) const    { return buttonMode;}


    float getImagingDepth_mm(void) const;
    DeviceIconType getIcon(void)              { return icon; }
    bool isBiDirectional(void)            { return biDirectional; }
    QString getDisclaimerText(void)       { return disclaimerText; }
    void setInternalImagingMask_px(int mask)  { internalImagingMask_px = mask; }
    float getPixelsPerUm() const {return pixelsPerUm;}
    DeviceIconType    icon;

public:
    static QString formatDeviceName(const QString& name);
    bool isAth() const {return m_isAth;}

    void setImagingDepth_mm(float value);

    void setALineLength_px(int value);

    QByteArray getDevicePropVersion() const;
    void setDevicePropVersion(const QByteArray &value);

    QByteArray getSledFwMinVersion() const;
    void setSledFwMinVersion(const QByteArray &value);

    QByteArray getIfFwMinVersion() const;
    void setIfFwMinVersion(const QByteArray &value);

    int getDefaultSpeedIndex() const;
    void setDefaultSpeedIndex(int value);

    QByteArray getDeviceCRC() const;
    void setDeviceCRC(const QByteArray &value);

    int getNumberOfSpeeds() const;
    void setNumberOfSpeeds(int value);

private:
    QString    deviceName;
    QString    splitDeviceName;
    QByteArray catheterType;
    QByteArray devicePropVersion{"3.0"};
    QByteArray sledFwMinVersion{"3.0"};
    QByteArray ifFwMinVersion{"3.0"};
    int        catheterLength;
    int        catheterRadius_um;
    int        internalImagingMask_px;
    int        biDirectional;
    int        numberOfSpeeds;
    int        revolutionsPerMin1;
    int        revolutionsPerMin2;
    int        revolutionsPerMin3;
    int        defaultSpeedIndex;
    int        aLineLength_px;
    int        measurementVersion;
    int        clockingEnabled;
    QByteArray clockingGain;
    QByteArray clockingOffset;

    QByteArray torqueLimit;
    QByteArray torqueTime;
    QByteArray stallBlinking;
    QByteArray buttonMode;

    float      imagingDepth_mm;
    float      pixelsPerMm;
    float      pixelsPerUm;
    QString    disclaimerText;
    QByteArray deviceCRC;
    bool       m_isAth;
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

    void setCurrentDevice( int devIndex );

    int getCurrentDevice( void ) { return currentDevice; };
    device *deviceAt( int devIndex ) { return( deviceList.at( devIndex ) ); }

    device *current(void);

    const QString& getCurrentDeviceName( void ) const;
    const QString& getCurrentSplitDeviceName( void ) const;
    QString getCurrentDeviceTitle( void ) const;

    QString getFileSystemSafeDeviceName( void );

    // Only way to easily iterate the list is to pass it off
    QList<device *>list( void ) { return deviceList; }

    bool loadDevice( QString deviceFile );
    bool checkVersion( QDomDocument *doc );

    void setBrightness (int value ) { brightness = value; }
    void setContrast( int value ) { contrast = value; }
    int  getBrightness (void ) { return brightness; }
    int  getContrast( void ) { return contrast; }

    bool getIsDeviceSimulation() const;
    void setIsDeviceSimulation(bool isDeviceSimulation);

    QImage *getSelectedIcon() const;
    void setSelectedIcon(QImage *selectedIcon);

signals:
    void sendWarning( QString );
    void sendFailure( QString ); // signal to consoleApp
    void displayMask( int );

public slots:
    void adjustMaskSize( int );

private:
    int currentDevice;
    QList<device *> deviceList;
    QSettings *settings;

    int brightness = 0;
    int contrast = 0;
    bool m_isDeviceSimulation{false};
    const QString m_unknownDeviceName{"Unknown"};

    deviceSettings();
    ~deviceSettings();

    deviceSettings(deviceSettings const &); // hide copy
    deviceSettings & operator=(deviceSettings const &); // hide assign
    QImage* m_selectedIcon{nullptr};
};
