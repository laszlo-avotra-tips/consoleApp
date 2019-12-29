/*
 * laser.h
 *
 * Interface for the laser
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */
#ifndef LASER_H_
#define LASER_H_

#include <QMutex>
#include <QObject>
#include <QSerialPort>
#include <QString>

/*
 * This namespace is used by the QSerialPort library.  Use the namespace
 * keyword to allow QSerialPort instead of TNX::QSerialPort
 */
using namespace TNX;

class Laser : public QObject
{
    Q_OBJECT

public:
    // Singleton
    static Laser & Instance()
    {
        static Laser theLaser;
        return theLaser;
    }

    void setPort( const char *port ) { portName = port; }
    void setConfig( const char *config ) { settings = config; }
    bool init( void );

signals:
    void diodeIsOn( bool );

public slots:
    bool isDiodeOn( void );
    void turnDiodeOn( void );
    void turnDiodeOff( void );
    void forceOff( void );

private:
    // amount of time to wait for responses
    static const int CommDelay_ms = 50;

    QSerialPort *serialPort;
    QString portName;
    QString settings;

    QMutex mutex;

    QByteArray getResponse( void );

                                        // prevent access to:
    Laser();                            //   default constructor
    ~Laser();                           //   default destructor
    Laser( Laser const & );             //   copy
    Laser & operator=( Laser const & ); //   assign
};

#endif // LASER_H_
