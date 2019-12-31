/*
 * sledsupport.cpp
 *
 * Authors: Sonia Yu, Ryan Radjabi
 *
 * Copyright (c) 2016-2018 Avinger, Inc.
 */
#include "sledsupport.h"
#include "defaults.h"
#include "logger.h"
#include "util.h"
#include "deviceSettings.h"
#include <QDebug>

/* Output from help command on a hyperterminal
 * Firmware Revision 0.06
 * Commands:
 * h          Print this list
 * so[1-3]    Select Ocelot Speeds 1-3
 * sr[0-1]    Set Run off,on
 * ssxxxx     Set Speed Constant to xxxx
 * sc[0-1]    Set Clocking off,on
 * scg[1-99]  Set Clocking Gain
 * sco[1-999] Set Clocking Offset
 * ga         Print All Parameters
 * gv         Print Sled version
 * gr         Print Running state
 * gc         Print Clocking mode
 * gs         Print Current Speed
 * go         Print Ocelot Speed
 */

// amount of time to wait for responses
static const int SledCommDelay_ms = 20;
static const int SledLoopDelay_ms = 200;

// update clocking mode every second
//static const int ClockingUpdateTimer_ms = 1000;

// Commands for accessing the Sled Support Board
const char SetClockingGain[]     = "scg";
const char SetClockingOffset[]   = "sco";
const char SetClockingMode[]     = "sc";
const char SetSpeed[]            = "ss";
const char SetSled[]             = "sr";
const char SetSledOn[]           = "sr1\r";
const char SetSledOff[]          = "sr0\r";
const char SetTorque[]           = "sto";
const char SetLimitTime[]        = "sti";
const char SetDirection[]        = "sd";
const char SetLimitBlink[]       = "sbl";
const char SetButtonMulti[]      = "sbm";
const char GetClockingGain[]     = "gcg\r";
const char GetClockingOffset[]   = "gco\r";
const char GetClockingMode[]     = "gc\r";
const char GetSpeed[]            = "gs\r";
const char GetTorque[]           = "gto\r";
const char GetLimitTime[]        = "gti\r";
const char GetFirmwareVersions[] = "gv\r";
const char GetRunningState[]     = "gr\r";

// Avaialble commands for future extention. Currently unused.
//const char GetCurrentSpeed[]     = "gs\r";
const char SetOcelotSpeed2[]     = "so2\r";
const char GetOcelotSpeed[]      = "go\r";
//const char GetRunningState[]     = "gr\r";

QByteArray resp;
QByteArray lastSpeed = "1000";
QByteArray oSpeed1;
QByteArray oSpeed2;
QByteArray oSpeed3;

/*
 * Constructor
 *
 * Set up defaults.  The real setup is done in init().
 */
SledSupport::SledSupport():m_speed(-1)
{
    isRunning        = false;
    prevClockingMode = SledSupport::UnknownMode;
    currClockingMode = SledSupport::UnknownMode;
    prevSledState    = SledSupport::UnknownState;
    currSledState    = SledSupport::UnknownState;
    comPort = "COMX";
	connect(this, SIGNAL(stopSledNow()), this, SLOT(stopSled()) );
}

/*
 * Destructor
 *
 * Close the port and free resources.
 */
SledSupport::~SledSupport()
{
    stop(); // end the running thread cleanly

//    if( ftHandle != NULL )
//    {
////        qDebug() << "Closing Serial Port";
//        FT_Close( ftHandle);
//    }
}

/*
 * General serial port write command for FTDI device
 */
bool SledSupport::writeSerial(QByteArray command)
{
    //qDebug() << "Command to write: " << command;
    bool retVal = true;
//    if( ftHandle != NULL )
//    {
//        int  cmdSize = command.size();
//        char* cmdData = new char(cmdSize);
//        memcpy(cmdData, command.data(), cmdSize);
//        DWORD bytesWritten;

//        ftStatus = FT_Write( ftHandle, cmdData, cmdSize, &bytesWritten );
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Could not write command" << command;
//            retVal = false;
//        }
//    }
//    else
//    {
//        qDebug() << "Serial Port not open";
//        retVal = false;
//    }
    return retVal;
}
/*
 * Initialize the Sled Support serial port hardware.
 */
bool SledSupport::init( void )
{
//    FT_DEVICE_LIST_INFO_NODE *ftdiDeviceInfo;
//    unsigned long ftdiDeviceCount = 0;
//    long COMPORT;
//    QString result = "COM port not found";

//    newDir = -1;
//    sledRunState = 0;       // stopped

//    bool status = true;
//    ftStatus = FT_CreateDeviceInfoList(&ftdiDeviceCount);
//    if( ftdiDeviceCount == 0 )
//    {
//        qDebug() << "No FTDI Devices !!!";
//        result = "No FTDI Devices !!!";
//        status = false;
//    }
//    else
//    {
////        qDebug() << "Number of FTDI Devices" << ftdiDeviceCount;
//        ftdiDeviceInfo = (FT_DEVICE_LIST_INFO_NODE*) malloc (sizeof(FT_DEVICE_LIST_INFO_NODE)*ftdiDeviceCount);
//        ftStatus = FT_GetDeviceInfoList( ftdiDeviceInfo, &ftdiDeviceCount );
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Could not get FTDI device list";
//            status = false;
//        }
//        if( ftdiDeviceCount > 1)
//        {
//            result = "Too many FTDI devices found";
//            status = false;
//        }

//        if( ftdiDeviceCount > 0)
//        {
//            ftHandle = ftdiDeviceInfo[0].ftHandle;
//            // make sure device is closed before we open it
//            ftStatus = FT_Close( ftHandle );
//            ftStatus = FT_Open( 0, &ftHandle );
//            if( ftStatus == FT_OK )
//            {
//                ftStatus = FT_GetComPortNumber(ftHandle, &COMPORT);
////                qDebug() << "FTDI COM port number" << COMPORT;
//                comPort.setNum(COMPORT);
//                result = "Found FTDI device at COM" + comPort;
//            }
//            else
//            {
//                qDebug() << "Could not open FTDI device";
//                status = false;
//            }
//            ftStatus = FT_Close( ftHandle );
//        }
//        else
//        {
//            result = "No FTDI devices found";
//            status = false;
//        }
//    }

//    if( status )
//    {
//        ftStatus = FT_Open(0, &ftHandle);    // open COM port
//        if( ftStatus != FT_OK )
//        {
//            // error opening COM port
//            displayWarningMessage( QString( tr( "Cannot open sled support serial port " ) ).append( comPort ) );
//            status = false;
//        }
//    }

//    if( status )
//    {
//        ftStatus = FT_SetBaudRate( ftHandle, 9600);
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Could not set baud rate";
//            status = false;
//        }
//        ftStatus = FT_SetDataCharacteristics( ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Could not set data charecteristics";
//            status = false;
//        }
//        ftStatus = FT_SetFlowControl( ftHandle, FT_FLOW_NONE, 0x11, 0x13 );
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Could not set flow control";
//            status = false;
//        }
//        ftStatus = FT_SetTimeouts( ftHandle, 50, 1000 );
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Could not set timeouts ";
//            status = false;
//        }
//        ftStatus = FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Purge failed";
//            status = false;
//        }
//    }
////    qDebug() <<  "SledSupport init: " << result;
//    return status;
    return true;//lcv
}

/*
 * Send the command to get the status of all variables.
 */
void SledSupport::getAllStatus()
{
//    if( ftHandle != NULL )
//    {
//        mutex.lock();

//        ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );
//        //qDebug() << "Tx:" << GetClockingMode;
//        writeSerial( GetClockingMode );
//        Sleep( SledCommDelay_ms );
//        QByteArray resp = getResponse();
//        //qDebug() << "Rx:" << resp;

//        //qDebug() << "Tx:" << GetClockingGain;
//        writeSerial( GetClockingGain );
//        Sleep( SledCommDelay_ms );
//        resp = getResponse();
//        //qDebug() << "Rx:" << resp;

//        //qDebug() << "Tx:" << GetClockingOffset;
//        writeSerial( GetClockingOffset );
//        Sleep( SledCommDelay_ms );
//        resp = getResponse();
//        //qDebug() << "Rx:" << resp;

//        mutex.unlock();
//    }
}

/*
 * Signal to subscribers the updated clocking mode.
 */
void SledSupport::updateClockingMode( void )
{
    if( currClockingMode != prevClockingMode )
    {
        emit announceClockingMode( currClockingMode );
        prevClockingMode = currClockingMode;
//        qDebug() << "Change in clocking mode. Mode:" << currClockingMode;
        LOG( INFO, QString( "Sled Support Board: Clocking mode change. Mode: %1" ).arg( currClockingMode ) );
    }
}

/*
 * handleSledResponse
 */
void SledSupport::handleSledResponse( void )
{
//    prevSledState = currSledState;

//    QByteArray resp = getResponse();

//    if( resp != "" ) // test against an empty string
//    {
//        if( resp.toUpper().contains( "ACK" ) )
//        {
//            currSledState = SledSupport::ConnectedState; // assume connected if ACK

//            int index = resp.toUpper().lastIndexOf( "GC" );
//            if( index != -1 )
//            {
//                if( resp.toUpper().contains( "1" ) )  // gc=1 if clocking is on
//                {
//                    currClockingMode = SledSupport::NormalMode;
//                }
//                else if( resp.toUpper().contains( "0" ) ) // gc=0 if clocking is off
//                {
//                    currClockingMode = SledSupport::DiagnosticMode;
//                }
//                else  // there is no proper signature in the bytes returned
//                {
//                    currClockingMode = SledSupport::UnknownMode;
//                    LOG( WARNING, QString( "Clocking mode is neither on nor off. Mode: %1, Response: %2" ).arg( currClockingMode ).arg( QString( resp ) ) );
//                }
//            }
//        }
//        else if (resp.toUpper().contains( "NAK") )
//        {
//            currClockingMode = SledSupport::UnknownMode;
//            currSledState = SledSupport::DisconnectedState; // assume disconnected if NAK
//        }
//        else // Something other than ACK or NAK or empty string.
//        {
//            currSledState = SledSupport::UnknownState; // assume it is disconnected
//            qDebug() << "Unknown response: " << resp;
//            LOG( WARNING, QString( "Unknown response from Serial port for clocking mode: %1" ).arg( QString( resp ) ) );
//        }
//    }

//    updateClockingMode();
}

/*
 * run
 *
 * This is the only code running in the new thread. It periodically loops around
 * to poll the Sled Support Board for clocking mode. If a re-connect state is
 * detected, it will push new clocking parameters to the Sled Support Board.
 * Also poll for Ocelot speed (for Ocelaris) every ~100 mSec.
 *
 * Poll every SledCommDelay_ms (100),
 * 1-9 get Ocelot speed
 * 10  get Clocking mode
 */
void SledSupport::run( void )
{
    int round = 0;
    QByteArray oSpeed = "1000";
    deviceSettings &dev = deviceSettings::Instance();
    LOG1(oSpeed);

    isRunning = true;
    while( isRunning )
    {
//        if( newDir >= 0 )
//        {
//            baParam.setNum( newDir );
//            newDir = -1;
//            setSledDirection( baParam );
//        }

////        if( round++ > 9 )
//        {
//            round = 0;
//            mutex.lock();
//            ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );
//            writeSerial( GetClockingMode );       // poll for the clocking mode

//            msleep( SledCommDelay_ms );                 // sleep to wait for a response

//            handleSledResponse();                   // parse the response and update mode
//            mutex.unlock();

//            // Get firmware version when state changes
//            if( currSledState != prevSledState)
//            {
//    //            qDebug() << "Sled state changed";
//                getFirmwareVersions(); // Read Sled and Sled Support firmware versions and announce to advanced view.
//            }

//            // Reinstate clocking parameters when a state change from Disconnected to Connected is detected.
//            if( prevSledState == SledSupport::DisconnectedState && currSledState == SledSupport::ConnectedState )
//            {
//    //            qDebug() << "Sled connected";
//                setSledParams( sledParams );
//            }
//        }
//        if ( dev.current()->isOcelaris() )
//        {
//            mutex.lock();
//            ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );
//            writeSerial( GetRunningState );     // find out if we are running and which direction
//            msleep( SledCommDelay_ms );                 // sleep to wait for a response
//            resp = getResponse();
////            qDebug() << "getRunning response: " << resp;
//            mutex.unlock();
//            if( resp != "" ) // test against an empty string
//            {
//                if( resp.toUpper().contains( "ACK" ) )
//                {
//                    sledRunState = dev.current()->getRotation();
//                    if( resp.toUpper().contains( "0" ) )  // not running
//                    {
//                        // stop things
//                        dev.current()->setRotation( -1 );
//						emit setDirButton(-1);
//                    }
////                    else qDebug() << "current direction: " << sledRunState << resp;
//                    if( resp.toUpper().contains( "1" ) )  // running CW (Passive)
//                    {
//                        if(sledRunState != 0)
//                        {
//                            dev.current()->setRotation( 0 );
//						}
//                            emit setDirButton(0);
////                        }
//                    }
//                    if( resp.toUpper().contains( "3" ) )  // running CCW (Active)
//                    {
//                        if(sledRunState != 1)
//                        {
//                            dev.current()->setRotation( 1 );
//						}
//                            emit setDirButton(1);
////                        }
//                    }
//                }
//            }

//            mutex.lock();
//            writeSerial( GetOcelotSpeed );       // poll for the Ocelot speed
//            msleep( SledCommDelay_ms );                 // sleep to wait for a response
//            resp = getResponse();
//            mutex.unlock();
//            if( resp != "" ) // test against an empty string
//            {
//                if( resp.toUpper().contains( "ACK" ) )
//                {
//                    if( resp.toUpper().contains( "1" ) )  // go=1
//                    {
//                        oSpeed = oSpeed1;
//                    }
//                    if( resp.toUpper().contains( "2" ) )  // go=2
//                    {
//                        oSpeed = oSpeed2;
//                    }
//                    if( resp.toUpper().contains( "3" ) )  // go=3
//                    {
//                        oSpeed = oSpeed3;
//                    }
//                    if( oSpeed != lastSpeed )
//                    {
//                        lastSpeed = oSpeed;
//                        int revsPerMin = oSpeed.toInt();
//                        int temp = ((1000*1200) / revsPerMin) - 4;
//                        int aLines = temp - temp%16;
//                        qDebug() << "*** Multi-speed setting:" << oSpeed << "Alines:" << aLines;
////						emit stopSledNow();
//                        emit changeDeviceSpeed(revsPerMin, aLines);
//                    }
//                }
//            }
//        }
//        else
        {
            msleep( SledLoopDelay_ms );     // sampling interval
        }
    }
}

void SledSupport::stopSled()
{
//	// Stop sled if running
//	QByteArray setOnSerialCmd = QByteArray( SetSled ).append( "0" ).append( "\r" );
//	mutex.lock();
//    LOG1(setOnSerialCmd);
//	//qDebug() << "Tx:" << setOnSerialCmd;
//	writeSerial( setOnSerialCmd );
//    Sleep( SledCommDelay_ms );
//    QByteArray resp = getResponse();
//	//qDebug() << "**** response: " << resp;
//    mutex.unlock();
}

void SledSupport::startSled()
{
//    deviceSettings &dev = deviceSettings::Instance();

//    sledParams.isHighSpeed = dev.current()->isHighSpeed();

//    // start the thread if a high speed device
//    if( sledParams.isHighSpeed )
//    {
//        sledParams.isEnabled   = dev.current()->isClockingEnabledByDefault();
//        sledParams.gain        = dev.current()->getClockingGain();
//        sledParams.offset      = dev.current()->getClockingOffset();
//        sledParams.speed.setNum( dev.current()->getRevolutionsPerMin() );
//        sledParams.torque      = dev.current()->getTorqueLimit();
//        sledParams.time.setNum( dev.current()->getTimeLimit() );
//        sledParams.blinkEnabled = dev.current()->getLimitBlink();
//        if(dev.current()->isOcelaris())
//        {
//            sledParams.sledMulti = 1;       // Click mode
//        }
//        else
//        {
//            sledParams.sledMulti = 0;       // Standard mode
//        }
//        LOG1(getSpeed());
//    }
//    LOG1(getSpeed());
//    setSledParams( sledParams );
}

void SledSupport::setSledRotation( int dir )
{
//    deviceSettings &dev = deviceSettings::Instance();
//    int direction = dev.current()->getRotation();
//    qDebug() << "**** sledsupport::setSledRotation()" << direction << dir;
//    if ( dev.current()->isBidirectional() && (dir != direction) )
//    {
//        qDebug() << "**** Setting new direction: " << dir;
//        newDir = dir;
//        dev.current()->setRotation( dir );
//    }
}

/*
 * Stop the thread by turning off isRunning and waiting
 * for run() to complete.
 */
void SledSupport::stop( void )
{
    isRunning = false;

    /*
     * 3000 ms timeout, this function will return false if it times out.
     * Consider handling the return value.
     */
    if( !wait( 3000 ) )
    {
        LOG( WARNING, "Sled Support stop() timeout (3000 ms) trying to exit the thread." );
    }
}

/*
 * setClockingParams
 */
void SledSupport::setSledParams( DeviceParams_T params )
{
    if( params.isHighSpeed )
    {
        setClockingMode( params.isEnabled );
        setClockingGain( params.gain );
        setClockingOffset( params.offset );
        setSledSpeed( params.speed );
        setSledTorque( params.torque );
        setSledLimitTime( params.time );
        setSledLimitBlink( params.blinkEnabled );
        setSledMultiMode( params.sledMulti );
    }
}

/*
 * updateDeviceForSledSupport
 *
 * This slot is executed in main and signaled by frontend. Call to set up
 * parameters for a new device. First completes run() by calling stop(),
 * then shared resources can be assumed safe for use.
 */
void SledSupport::updateDeviceForSledSupport()
{
//    qDebug() << "**** Sledsupport::updateDeviceForSledSUpport()";
    if( isRunning )
    {
        stop(); // blocks until run() completes
    }

    deviceSettings &dev = deviceSettings::Instance();

    sledParams.isHighSpeed = dev.current()->isHighSpeed();

    // start the thread if a high speed device
    if( sledParams.isHighSpeed )
    {
        sledParams.isEnabled   = dev.current()->isClockingEnabledByDefault();
        sledParams.gain        = dev.current()->getClockingGain();
        sledParams.offset      = dev.current()->getClockingOffset();
        sledParams.speed.setNum( dev.current()->getRevolutionsPerMin() );
        sledParams.torque      = dev.current()->getTorqueLimit();
        sledParams.time.setNum( dev.current()->getTimeLimit() );
        sledParams.blinkEnabled = dev.current()->getLimitBlink();
        if(dev.current()->isOcelaris())
        {
            sledParams.sledMulti = 1;       // Click mode
        }
        else
        {
            sledParams.sledMulti = 0;       // Standard mode
        }

        getFirmwareVersions(); // get versions from SSB and send to advanced view
        //qDebug() << "Sled version; " << sledParams.vSled << "Sled Support version: " << sledParams.vSSB;

        setSledParams( sledParams );
        if (!dev.current()->isBidirectional())
        {
            newDir = 0;    // make sure we start CW if not Ocelaris
            dev.current()->setRotation( -1 );      // Don't display center
        }
		else
		{
            // get the Ocelaris speeds
            oSpeed1 = dev.current()->getSpeed1();
            oSpeed2 = dev.current()->getSpeed2();
            oSpeed3 = dev.current()->getSpeed3();
			qDebug() << "**** Multi-speeds" << oSpeed1 << oSpeed2 << oSpeed3;
		}
        start();
    }
    else // low speed device
    {
        currClockingMode = SledSupport::NotApplicableMode;
        updateClockingMode();
    }
}

/*
 * Send command to set clocking correction ON/OFF.
 */
void SledSupport::setClockingMode( bool mode )
{
    QByteArray compareVal;
    if( ftHandle != NULL )
    {
        mutex.lock();

        compareVal = QByteArray( "gc=" ).append( QString::number( mode ) );
        QByteArray setClockingSerialCmd = QByteArray( SetClockingMode ).append( QString::number( mode ) ).append( "\r" );
        //qDebug() << "Tx:" << setClockingSerialCmd;
        writeSerial( setClockingSerialCmd );

        LOG( INFO, QString( "Sled Support Board: set clocking mode: %1" ).arg( QString::number( mode ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        //qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
 //           qDebug() << "set clocking returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set clocking returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

/*
 * Send command to set clocking gain.
 */
void SledSupport::setClockingGain( QByteArray gain )
{
    QByteArray setGainSerialCmd = QByteArray( SetClockingGain ).append( gain ).append( "\r" );
    if( ftHandle != NULL )
    {
        mutex.lock();

        //qDebug() << "Tx:" << setGainSerialCmd;
        writeSerial( setGainSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set clocking gain: %1" ).arg( QString( gain ) ) );
        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        //qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
//            qDebug() << "set clocking gain returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set clocking gain returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

/*
 * Send command to set clocking offset.
 */
void SledSupport::setClockingOffset( QByteArray offset )
{
    QByteArray setOffsetSerialCmd = QByteArray( SetClockingOffset ).append( offset ).append( "\r" );
    if( ftHandle != NULL )
    {
        mutex.lock();

        //qDebug() << "Tx:" << setOffsetSerialCmd;
        writeSerial( setOffsetSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set clocking offset: %1" ).arg( QString( offset ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        //qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
//            qDebug() << "set clocking offset returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set clocking offset returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

/*
 * setSledSpeed
 */

void SledSupport::setSledSpeed(QByteArray speed)
{
    if( ftHandle != NULL )
    {
        LOG1(speed.toInt());
        auto speedInt = speed.toInt();
//        if(speedInt != m_speed)
        {
            m_speed = speedInt;
            QByteArray setSpeedSerialCmd = QByteArray( SetSpeed ).append( speed ).append( "\r" );
            mutex.lock();
            qDebug() << "Tx:" << setSpeedSerialCmd;
            writeSerial( setSpeedSerialCmd );
            LOG( INFO, QString( "Sled Support Board: set speed: %1" ).arg( QString( speed ) ) );
            Sleep( SledCommDelay_ms );
            QByteArray resp = getResponse();
            mutex.unlock();
            LOG1(m_speed);
            emit speedChanged(m_speed);

    //        qDebug() << "Rx:" << resp;
            if( resp.toUpper().contains( "NAK" ) )
            {
                qDebug() << "set speed returned NAK" << resp.toUpper();
                LOG( WARNING, QString( "Sled Support Board: set speed returned NAK. Response: %1" ).arg( QString( resp ) ) );
            }
        }
    }else{
        LOG1(ftHandle);
    }

}

/*
 * set Sled Direction
*/
void SledSupport::setSledDirection( QByteArray dir )
{
//    if( ftHandle != NULL )
//    {
//        qDebug() << "*** sledsupport::setDirection(): " << dir;
//        bool running = false;
//        // first get current run mode
//        mutex.lock();
//        ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );   // flush input buffer
//        if( ftStatus != FT_OK )
//        {
//            qDebug() << "Input flush failed";
//        }
//        writeSerial( GetRunningState );
//        msleep( SledCommDelay_ms );                 // sleep to wait for a response
//        QByteArray resp = getResponse();
//        mutex.unlock();
//        qDebug() << "get running state response:" << resp;
//        if(( resp.toUpper().contains( "1" )) || ( resp.toUpper().contains( "3" ))) running = true;

//        // remember direction in case Sled of off-line
//        sledParams.dir =  dir ;

//        QByteArray setDirSerialCmd = QByteArray( SetDirection ).append( dir ).append( "\r" );

//        mutex.lock();
//        //qDebug() << "Tx:" << setDirSerialCmd;
//        writeSerial( setDirSerialCmd );
//        msleep( SledCommDelay_ms );                 // sleep to wait for a response
//        resp = getResponse();
//        mutex.unlock();
////        qDebug() << "set direction response:" << resp;

//        //qDebug() << "Rx:" << resp;
//        if( resp.toUpper().contains( "NAK" ) )
//        {
//            qDebug() << "set direction returned NAK" << resp.toUpper();
//            LOG( WARNING, QString( "Sled Support Board: set direction returned NAK. Response: %1" ).arg( QString( resp ) ) );
//        }

//        if( running )
//        {
//            msleep( 500 );                 // make sure the Sled is stopped
//            QByteArray setOnSerialCmd = QByteArray( SetSled ).append( "1" ).append( "\r" );
//            mutex.lock();
//            LOG1(setOnSerialCmd);
////            qDebug() << "Tx:" << setOnSerialCmd;
//            writeSerial( setOnSerialCmd );
//            msleep( SledCommDelay_ms );                 // sleep to wait for a response
//            mutex.unlock();
//        }
//        deviceSettings &dev = deviceSettings::Instance();
//        dev.current()->setRotation( dir.toInt() );    // flip image if CCW
//    }
}

/*
 * setSledTorque
 */

void SledSupport::setSledTorque(QByteArray torque)
{
    int temp = (int) (torque.toFloat() * 10);   // Sled expects parameter without the decimal point
    torque.setNum( temp );
    QByteArray setTorqueSerialCmd = QByteArray( SetTorque ).append( torque ).append( "\r" );
    if( ftHandle != NULL )
    {
        mutex.lock();

//        qDebug() << "Tx:" << setTorqueSerialCmd;
        writeSerial( setTorqueSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set torque limit: %1" ).arg( QString( torque ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

//        qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
//            qDebug() << "set torque returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set torque returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

/*
 * setSledLimitTime
 */

void SledSupport::setSledLimitTime(QByteArray limit)
{
    QByteArray setLimitSerialCmd = QByteArray( SetLimitTime ).append( limit ).append( "\r" );
    if( ftHandle != NULL )
    {
        mutex.lock();

//        qDebug() << "Tx:" << setLimitSerialCmd;
        writeSerial( setLimitSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set torque timeout: %1" ).arg( QString( limit ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

//        qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
//            qDebug() << "set limit returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set limit returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

/*
 * setSledLimitBlink
 */

void SledSupport::setSledLimitBlink(int blink)
{
    if( ( blink != 0 ) || ( blink != 1 ) )
    {
        return;
    }

    char isBlinkEnabled = '1';
    if(!blink)
    {
        isBlinkEnabled = '0';
    }
    QByteArray setBlinkSerialCmd = QByteArray( SetLimitBlink ).append( isBlinkEnabled ).append( "\r" );
    if( ftHandle != NULL )
    {
        mutex.lock();

//        qDebug() << "Tx:" << setBlinkSerialCmd;
        writeSerial( setBlinkSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set blink: %1" ).arg( char( isBlinkEnabled ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

//        qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
//            qDebug() << "set blink returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set blink returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

void SledSupport::setSledMultiMode(int mode)
{
    baParam.setNum( mode );

    QByteArray setSledMultiSerialCmd = QByteArray( SetButtonMulti ).append( baParam ).append( "\r" );
    if( ftHandle != NULL )
    {
        mutex.lock();

        qDebug() << "Tx:" << setSledMultiSerialCmd;
        writeSerial( setSledMultiSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set mode: %1" ).arg( mode ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

//        qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
            qDebug() << "set mode returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set mode returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

/*
 * Poll the SSB for firmware versions using "gv".
 * The response will be in the format "ACK gv=01.02_03.14",
 * Sled: Major version: 01
 *       Minor version: 02
 * SSB:  Major version: 03
 *       Minor version: 14
 */
void SledSupport::getFirmwareVersions( void )
{
//	qDebug() << "**** Sledsupport::getFirmwareVersions";
    deviceSettings &dev = deviceSettings::Instance();
    if( ftHandle != NULL )
    {
		resp = getResponse();			// clear any leftovers
        mutex.lock();
        writeSerial( GetFirmwareVersions );
        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        QString strPrefix = "ACK gv=";
        QString versionTemplate = "xx.xx";
        QByteArray sledVersion = "N/A";
        QByteArray ssbVersion  = "N/A";

        /*
         * Announce the firmware version if the correct response can be parsed,
         * display "N/A" if not. The version strings are messaged to advanced view
         * via the path through frontend using signals & slots.
         */
        if( resp.startsWith( strPrefix.toLatin1() ) )
        {
            // parse the response
            resp.remove( 0, strPrefix.length() ); // remove 7 characters "ACK gv="

            sledVersion = qualifyVersion( resp.left( versionTemplate.length() ) );
            ssbVersion  = qualifyVersion( resp.right( versionTemplate.length() ) );
        }
        sledParams.vSled = sledVersion;
        sledParams.vSSB  = ssbVersion;
//		qDebug() << "**** Announce firmware versions";
        emit announceFirmwareVersions( sledVersion, ssbVersion );
        LOG( INFO, QString( "Firmware versions: Sled - %1, Sled Support Board - %2" ).arg( QString( sledVersion ) ).arg( QString( ssbVersion ) ) );

        if( sledParams.vSled.startsWith( "1.") )
        {
            qDebug() << "Sled Version is 1.X, is not supported";
            LOG( INFO, QString( "Sled Version %1 is not supported" ).arg( QString( sledParams.vSled ) ) );
            emit handleError( tr("Sled Version %1 is not supported - Require Version 2.3 or higher\nWill Shutdown Case").arg( QString( sledParams.vSled ) ) );
            return;
        }
        if( sledParams.vSSB.startsWith( "1.") || sledParams.vSSB.startsWith( "2.") || sledParams.vSSB.startsWith( "3.0") )
        {
            qDebug() << "Sled Support Board Version is 1.X, 2.X or 3.0 is not supported";
            LOG( INFO, QString( "Sled Support Board Version %1 is not supported" ).arg( QString( sledParams.vSSB ) ) );
            emit handleError( tr("Sled Support Board Version %1 is not supported - Require Version 3.1 or higher\nWill Shutdown Case").arg( QString( sledParams.vSSB ) ) );
            return;
        }
        if(dev.current()->isOcelaris())
        {
            if( sledParams.vSled.startsWith( "1.") || sledParams.vSled.startsWith( "2.") )
            {
                qDebug() << "Ocelaris requires Sled Version 3.X";
                LOG( INFO, QString( "Ocelaris requires Sled Version 3.X" ).arg( QString( sledParams.vSled ) ) );
                emit handleError( tr("Ocelaris requires Sled Version 3.0 or higher \nWill Shutdown Case").arg( QString( sledParams.vSled ) ) );
                return;
            }
        }
    }
}

/*
 * qualifyVersion
 *
 * Take in a version in the format NN.NN and return a byte array "N/A" if
 * this ByteArray is literally "xx.xx". If not, trim the leading zeros and return
 * the ByteArray, so "05.01" should return "5.1". This should not trim the leading
 * zeros if the major or minor version is "00", otherwise version "04.00" would be
 * transformed to "4." when we really want "4.0".
 *
 * Unit Tests:
 *   qDebug() << "00.00 qualifies to: " << qualifyVersion( "00.00" );
 *   qDebug() << "10.10 qualifies to: " << qualifyVersion( "10.10" );
 *   qDebug() << "99.00 qualifies to: " << qualifyVersion( "99.00" );
 *   qDebug() << "01.10 qualifies to: " << qualifyVersion( "01.10" );
 *   qDebug() << "10.01 qualifies to: " << qualifyVersion( "10.01" );
 */
QByteArray SledSupport::qualifyVersion( QByteArray v )
{
    QByteArray newV;
    // check for valid sled version
    if( v.startsWith( "xx.xx" ) )
    {
        newV = "N/A";
    }
    else
    {
        QString vMajor = v.mid( 0, 2 );
        if( vMajor.startsWith( "0" ) )
        {
            vMajor.remove( 0, 1 );
        }

        QString vMinor = v.mid( 3, 2 );
        if( vMinor.startsWith( "0" ) )
        {
            vMinor.remove( 0, 1 );
        }

        // recombine the version
        newV.clear();
        newV = vMajor.toLatin1();
        newV.append( "." );
        newV.append( vMinor.toLatin1() );
    }

    return newV;
}

int SledSupport::getSpeed() const
{
    return m_speed;
}

/*
 * Read available data from the serial port.
 */
QByteArray SledSupport::getResponse( void )
{
     QByteArray data;
//     DWORD bytesToRead = 256;
//     DWORD bytesRead;
//     char buffer[256];

//     //qDebug() << "Reading Sled Response";

//     ftStatus = FT_Read( ftHandle, buffer, bytesToRead, &bytesRead );
//     if( ftStatus != FT_OK )
//     {
//         qDebug() << "Serial read failed";
//     }
//     else
//     {
//         buffer[bytesRead] = '\0';
//         data = buffer;
//         data = data.simplified();
//     }
     return data;
}
