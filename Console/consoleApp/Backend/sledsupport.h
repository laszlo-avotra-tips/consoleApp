/*
 * sledsupport.h
 *
 * Authors: Sonia Yu, Ryan Radjabi
 *
 * Copyright (c) 2016-2018 Avinger, Inc.
 */
#ifndef SLEDSUPPORT_H
#define SLEDSUPPORT_H

#include <QMutex>
#include <QObject>
#include <QThread>
//#include <QSerialPort>
#include <QString>
#include "ftd2xx.h"

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

    struct DeviceParams_T
    {
        bool       isHighSpeed;
        bool       isEnabled;
        QByteArray gain;
        QByteArray offset;
        QByteArray speed;
        QByteArray torque;
        QByteArray time;
        QByteArray dir;
        int        blinkEnabled;
        int        sledMulti;
        QByteArray vSled;
        QByteArray vSSB;
    };

    QByteArray baParam;
    void stop( void );
    void setSledSpeed( QByteArray speed);

    int getSpeed() const;

signals:
    void announceClockingMode( int );
    void announceFirmwareVersions( QByteArray, QByteArray );
    void handleError( QString );
    void changeDeviceSpeed( int, int );
	void stopSledNow();
    void setDirButton( int );
    void speedChanged(int);

public slots:
    void updateDeviceForSledSupport();
    void getAllStatus();
    void setSledRotation( int );
	void stopSled();
    void startSled();

protected:
    void run( void );

private slots:

private:
//    void stop( void );
    void setSledParams( DeviceParams_T params );
    bool writeSerial(QByteArray command);

    QMutex mutex;

    QByteArray getResponse( void );
    void updateClockingMode( void );
    void setClockingMode( bool mode );
    void setClockingGain( QByteArray gain );
    void setClockingOffset( QByteArray offset );
    void setSledDirection( QByteArray dir);
    void setSledTorque( QByteArray torque);
    void setSledLimitTime( QByteArray torque);
    void setSledLimitBlink( int blinkEnabled);
    void setSledMultiMode( int mode);
    void handleSledResponse( void );
    void getFirmwareVersions( void );
    QByteArray qualifyVersion( QByteArray v );

    int newDir;
    int sledRunState;

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

    DeviceParams_T sledParams;

    QString comPort;
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;

    ClockingMode_e currClockingMode;
    ClockingMode_e prevClockingMode;
    bool isRunning;
    SledState_e currSledState;
    SledState_e prevSledState;
    int m_speed;

                                                    // prevent access to:
    SledSupport();                                  //   default constructor
    ~SledSupport();                                 //   default destructor
    SledSupport( SledSupport const & );             //   copy
    SledSupport & operator=( SledSupport const & ); //   assign
};

#endif // SLEDSUPPORT_H
