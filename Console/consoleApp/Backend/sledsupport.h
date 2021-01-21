/*
 * sledsupport.h
 *
 * Authors: Sonia Yu, Ryan Radjabi
 *
 * Copyright (c) 2016-2017 Avinger, Inc.
 */
#ifndef SLEDSUPPORT_H
#define SLEDSUPPORT_H

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QString>
#include "ftd2xx.h"

//using namespace TNX;

class SledSupport : public QThread
{
    Q_OBJECT

public:

    // Singleton
    static SledSupport & Instance()
    {
        static SledSupport theBoard;
        return theBoard;
    }

    // Initialize serial communication
    bool init( void );
    void setSledSpeed( QByteArray );
    int runningState();
    int lastRunningState() const;
    void enableDisableBidirectional();
    bool writeSerial(QByteArray command);

    struct DeviceClockingParams_T
    {
        int        isEnabled;
        QByteArray gain;
        QByteArray offset;
        QByteArray speed;
        QByteArray torque;
        QByteArray time;
        QByteArray limit;
        QByteArray vSled;
        QByteArray vSSB;
        QByteArray dir;
        int blinkEnabled;
    };

    int newMode;
    int newOffset;
    int newGain;
    int newSpeed;
    int newDevice;
    int newDir;
    QByteArray baParam;

    int getLastRunningState() const;
    void toggleDirection();

    bool getIsClockwise() const;

signals:
    void announceClockingMode( int );
    void announceFirmwareVersions( QByteArray, QByteArray );
    void announceSpeed( QByteArray );
    void handleError( QString );
    void setSlider( int );
    void setOffsetSlider( int );
    void setGainSlider( int );
    void setCorrection( int );

public slots:
//    void updateDeviceForSledSupport( bool, QByteArray, QByteArray, int, QByteArray, QByteArray );
    void setMode( int );
    void setOffset( int );
    void setGain( int );
    void setSpeed( int );
    void setDevice( int );
    void setPower( int count );

protected:
    void run( void );

private slots:

private:
    void stop( void );
    void setClockingParams( DeviceClockingParams_T params );

    QMutex mutex;

    QByteArray getResponse( void );
    void updateClockingMode( void );
    void setClockingMode( int mode );
    void setClockingGain( QByteArray gain );
    void setClockingOffset( QByteArray offset );
    void setSledTorqueLimit( QByteArray );
    void setSledTimeLimit( QByteArray );
    void setSledLimitBlink( int );
    void handleClockingResponse( void );
    void getFirmwareVersions( void );
    QByteArray qualifyVersion( QByteArray v );

    QString commandToString(const QByteArray& ba);

    enum ClockingMode_e
    {
        DiagnosticMode = 0,  // clocking is off
        NormalMode,          // clocking is on
        UnknownMode,         // Can not tell which clocking mode is in
        NotApplicableMode    // use for low speed devices
    };

    enum SledState_e
    {
        UnknownState = 0,
        DisconnectedState,
        ConnectedState
    };

    FT_STATUS ftStatus;
    FT_DEVICE_LIST_INFO_NODE *ftdiDeviceInfo;
    FT_HANDLE ftHandle;

    DeviceClockingParams_T sledParams;

    ClockingMode_e currClockingMode;
    ClockingMode_e prevClockingMode;
    bool isRunning;
    SledState_e currSledState;
    SledState_e prevSledState;
    int m_lastRunningState;
    bool m_isClockwise{true};

                                                    // prevent access to:
    SledSupport();                                  //   default constructor
    ~SledSupport();                                 //   default destructor
    SledSupport( SledSupport const & );             //   copy
    SledSupport & operator=( SledSupport const & ); //   assign

};

#endif // SLEDSUPPORT_H
