/*
 * laser.cpp
 *
 * Interface to the laser
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */
#include <QThread>
#include <QDebug>
#include "defaults.h"
#include "laser.h"
#include "logger.h"
#include "util.h"
#include "ioController.h"
#include "logger.h"

// TBD: make this the base class.  Create classes for Santec and General Photonics

/*
 *
 * Written specifically for the Santec OEM HSL-200-H
 * COM:  9600 8N1
 *       X-control = None
 *       Delimiter = CR (0x0A, \r)
 *       Standard RS-232 cable (not null modem)
 *
 * Commands:
 *    LO  turn on  the laser diode
 *    LF  turn off the laser diode
 *    SU  sets output variable to status (seems superfluous)
 *    DI  requests data; output data of the previously executed command
 *
 * Commands are not case-sensitive.
 * The LO and LF commands echo themselves.
 * The SU command will echo the last thing sent from the laser.
 * The DI command will return the status data. Status data is 8 bytes:
 *    1     : LD Code  [blank] = laser diode off
 *                     [minus] = laser diode on
 *    2 - 4 : 0 (not used)
 *    5     : LD block temperature control status.  The temperature is out of
 *            range ~5 seconds when changing LD states
 *               0 = temperature is within range (+/- 0.1 deg to set value)
 *               1 = temperature is out of range
 *    6 - 8 : 0 (not used)
 *
 *    Example status:  -0001000  Current is supplied to the LD and the LD
 *                               temperature is out of range
 *
 *    Example usage:  '<' Commands sent from software (lowercase)
 *                    '>' Responses from the laser
 *    < lo
 *    > LO
 *    < su
 *    > LO
 *    < di
 *    > -0000000
 *    < su
 *    > -0000000
 *    < lf
 *    > LF
 *    < su
 *    > LF
 */

const QString LaserWarningMessage( QObject::tr( "Unable to talk to the laser.\n\nPlease contact Avinger Service at %1." ).arg( ServiceNumber ) );

// Commands for accessing the Santec laser
const char DiodeOn[]       = "LO\r";
const char DiodeOff[]      = "LF\r";
const char LaserStatus[]   = "DI\r";


/*
 * Constructor
 *
 * Set up defaults.  The real work is done in init().
 */
Laser::Laser()
{
    portName = DefaultPortName;
    settings = DefaultLaserCommConfig;
}


/*
 * Destructor
 *
 * Shutdown the laser and free resources.
 */
Laser::~Laser()
{
    if( serialPort != NULL )
    {
        // The system may not have logging services available anymore
        forceOff();
        serialPort->close();

        delete serialPort;
        serialPort = NULL;
    }
}


/*
 * init
 *
 * Initialize the serial port hardware
 */
bool Laser::init( void )
{
    bool status = true;

    // create serial port object
//lcv    serialPort = new QSerialPort( portName, settings );
    serialPort = new QSerialPort( portName );

//    if( serialPort == NULL )
//    {
//        // error:  failures stop the application and do not return
//        displayFailureMessage( QString( tr( "Cannot create accessor to serial port " ) ).append( qPrintable( portName ) ), true );
//        status = false;
//    }

//    // open COM port
//    else if( !serialPort->open() )
//    {
//        // error:  failures stop the application and do not return
//        displayFailureMessage( QString( tr( "Cannot open serial port " ) ).append( qPrintable( portName ) ), true );
//        status = false;
//    }

//    else if( !serialPort->setCommTimeouts( QSerialPort::CtScheme_NonBlockingRead ) )
//    {
//        displayWarningMessage( QString( tr( "Cannot set communications timeout values at port " ) ).append( qPrintable( portName ) ) );
//        status = false;
//    }

//    // The Santec laser does not use flow control
//    serialPort->setFlowControl( QPortSettings::FLOW_OFF );

//    // flush any data sitting in the buffers
//    serialPort->flushInBuffer();
//    serialPort->flushOutBuffer();
//    serialPort->bytesAvailable();

//    ioController &ioc = ioController::Instance();

//    // Verify that the IO Controller is ready before talking to the Laser
//    if( ioc.isReady() )
//    {
//        // Verify that we can talk to the laser
//        isDiodeOn();
//    }
//    else
//    {
//        displayWarningMessage( tr( "Attempting to access the Laser before IO Controller is ready" ) );
//        status = false;
//    }

    return status;
}

/*
 * diodeOn
 *
 * Send the command to turn the diode on
 */
void Laser::turnDiodeOn()
{
    if( serialPort != NULL )
    {
        mutex.lock();
        serialPort->write( DiodeOn );
        mutex.unlock();
        LOG( INFO, "LASER: Diode ON" );
    }

    // eat the echo
    getResponse();

    // verify the laser diode turned on
    if( !isDiodeOn() )
    {
        displayWarningMessage( LaserWarningMessage );
    }
}

/*
 * diodeOff
 *
 * Send the command to turn the diode off
 */
void Laser::turnDiodeOff()
{
    if( serialPort != NULL )
    {
        mutex.lock();
        serialPort->write( DiodeOff );
        mutex.unlock();
        LOG( INFO, "LASER: Diode OFF" );
    }

    // eat the echo
    getResponse();

    // verify the laser diode turned off
    if( isDiodeOn() )
    {
        displayWarningMessage( LaserWarningMessage );
    }
}


/*
 * forceOff
 *
 * Send the command to turn the diode off without any logging and checking.  This is a
 * best-effort attempt to disable the laser.  It's main use-case is when the system
 * is failing.
 */
void Laser::forceOff()
{
    if( serialPort != NULL )
    {
        mutex.lock();
        serialPort->write( DiodeOff );
        mutex.unlock();
    }
}

/*
 * isDiodeOn
 *
 * Get the status of the diode.  This slot emits the status of the diode for any listening
 * objects and also returns the value in case it is called directly.
 */
bool Laser::isDiodeOn()
{
    // Clear anything in the buffer (e.g., previous ON/OFF echo)
//lcv    serialPort->flushInBuffer();
    serialPort->flush();

#if DEBUG_LASER
    qDebug() << "Status:";
#endif
    mutex.lock();
    serialPort->write( LaserStatus );
    mutex.unlock();

#if DEBUG_LASER
    qDebug() << "isDiodeOn() Bytes available:" << serialPort->bytesAvailable();
#endif

    QByteArray resp = getResponse();

    // if byte 0 = <space>, status = false (off)
    //           =    -   , status = true  (on)
    // TBD: Make status  a tri-state
    bool status = false;
    if( resp.length() > 0 )
    {
        // Verify we have a real response from the laser
        if( resp.at( 0 ) == '-' )
        {
            status = true;
        }
        else if( resp.at( 0 ) == ' ' )
        {
            status = false;
        }
        else
        {
            displayWarningMessage( LaserWarningMessage );
        }
    }
    else
    {
        displayWarningMessage( LaserWarningMessage );
    }

#if DEBUG_LASER
    qDebug() << "Laser isDiodeOn:" << status;
#endif
    emit diodeIsOn( status );
    return( status );
}

#ifndef Q_WS_WIN
void Sleep(int ms) {
//lcv	usleep(ms * 1000);
    QThread::msleep(ms);
}
#endif

/*
 * getResponse
 *
 * Read available data from the serial port
 */
QByteArray Laser::getResponse( void )
{
    // wait for data to come back
    Sleep( CommDelay_ms );

    quint64 bytesAvailable = serialPort->bytesAvailable();
#if DEBUG_LASER
    qDebug() << "getResponse() Bytes available:" << bytesAvailable;
#endif

    mutex.lock();
    QByteArray data = serialPort->read( bytesAvailable );
    mutex.unlock();

#if DEBUG_LASER
    qDebug() << "  Response: " << data.toUpper();
#endif

    /*
     * Log any NR responses from the laser.  These were seen sporadically at
     * startup during development but never lasted more than one request.
     * See #597.
     */
    if( data.toUpper().startsWith( "NR" ) )
    {
        LOG( INFO, "LASER: Laser responded with NR" );
        qDebug() << "LASER: Laser responded with NR";
    }

    return data;
}
