/*
 * sledsupport.cpp
 *
 * Authors: Sonia Yu, Ryan Radjabi
 *
 * Copyright (c) 2016-2017 Avinger, Inc.
 */
#include "sledsupport.h"
#include "defaults.h"
#include "QString"
#include <QDebug>
#include "deviceSettings.h"
#include "logger.h"

#include <QTextCodec>

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
namespace{

// amount of time to wait for responses
static const int SledCommDelay_ms = 100;

// update clocking mode every second
static const int ClockingUpdateTimer_ms = 1000;

// Commands for accessing the Sled Support Board
QByteArray SetClockingGain       = "scg";
QByteArray SetClockingOffset     = "sco";
QByteArray SetClockingMode       = "sc";
QByteArray SetSpeed              = "ss";
QByteArray SetTorque             = "sto";
QByteArray SetLimitTime          = "sti";
QByteArray SetDirection          = "sd";
QByteArray SetLimitBlink         = "sbl";
QByteArray GetClockingGain       = "gcg\r";
QByteArray GetClockingOffset     = "gco\r";
QByteArray GetClockingMode       = "gc\r";
QByteArray GetSpeed              = "gs\r";
QByteArray GetTorque             = "gto\r";
QByteArray GetLimitTime          = "gti\r";
QByteArray GetFirmwareVersions   = "gv\r";
QByteArray GetRunningState       = "gr\r";
QByteArray SetSledOn             = "sr1\r";
QByteArray SetSledOff            = "sr0\r";
QByteArray SetPower              = "spw";

// Avaialble commands for future extention. Currently unused.
QByteArray GetOcelotSpeed        = "go\r";
QByteArray SetOcelotSpeed        = "so";

const std::map<QString,QString> commandLut
{
    {{QString("scg")},{QString("SetClockingGain "   )}},
    {{QString("sco")},{QString("SetClockingOffset " )}},
    {{QString("sto")},{QString("SetTorque "         )}},
    {{QString("sti")},{QString("SetLimitTime "      )}},
    {{QString("sbl")},{QString("SetLimitBlink "     )}},
    {{QString("gcg")},{QString("GetClockingGain "   )}},
    {{QString("gco")},{QString("GetClockingOffset " )}},
    {{QString("gto")},{QString("GetTorque "         )}},
    {{QString("gti")},{QString("GetLimitTime "      )}},
    {{QString("sr1")},{QString("SetSledOn "         )}},
    {{QString("sr0")},{QString("SetSledOff "        )}},
    {{QString("spw")},{QString("SetPower "          )}},
    {{QString("sc")},{QString("SetClockingMode "    )}},
    {{QString("ss")},{QString("SetSpeed "           )}},
    {{QString("sd")},{QString("SetDirection "       )}},
    {{QString("gc")},{QString("GetClockingMode "    )}},
    {{QString("gs")},{QString("GetSpeed "           )}},
    {{QString("gv")},{QString("GetFirmwareVersions ")}},
    {{QString("gr")},{QString("GetRunningState "    )}}
};

}

/*
 * Constructor
 *
 * Set up defaults.  The real setup is done in init().
 */
SledSupport::SledSupport()
{
    isRunning        = false;
    prevClockingMode = SledSupport::UnknownMode;
    currClockingMode = SledSupport::UnknownMode;
    prevSledState    = SledSupport::UnknownState;
    currSledState    = SledSupport::UnknownState;
}

/*
 * Destructor
 *
 * Close the port and free resources.
 */
SledSupport::~SledSupport()
{
    stop(); // end the running thread cleanly

#if USE_SLED_SUPPORT_BOARD
    if( ftHandle != NULL )
    {
#if USE_NEW_SLED_SUPPORT_BOARD
        ftStatus = FT_SetBitMode( ftHandle, 0xF0, 0x20 );   // turn power off Sled Support and Sled
#endif
        qDebug() << "Closing Serial Port";
        FT_Close( ftHandle);
    }
#endif
}

/*
 * General serial port write command for FTDI device
 */
bool SledSupport::writeSerial(QByteArray command)
{
    //qDebug() << "Command to write: " << command;
    if(command != GetRunningState){
        LOG( INFO, QString( "Sled Support Board: writeSerial command: %1 " ).arg( commandToString(command) ) );
    }
    bool retVal = true;
    if( ftHandle != NULL )
    {
        int  cmdSize = command.size();
        char* cmdData = new char(cmdSize);
        memcpy(cmdData, command.data(), cmdSize);
        DWORD bytesWritten;


        //ftStatus = FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
        if( ftStatus != FT_OK )
        {
            qDebug() << "Purge failed";
        }
        ftStatus = FT_Write( ftHandle, cmdData, cmdSize, &bytesWritten );
        if( ftStatus != FT_OK )
        {
            qDebug() << "Could not write command" << command;
            LOG( WARNING, QString( "Sled Support Board: writeSerial could not write command: %1 " ).arg( commandToString(command) ) );
            retVal = false;
        }
        else
        {
            qDebug() << "Serial bytes written: " << bytesWritten;
            if(command != GetRunningState){
                LOG( INFO, QString( "Sled Support Board: writeSerial bytes written: %1 " ).arg( bytesWritten ) );
            }
        }
    }
    else
    {
        qDebug() << "Serial Port not open";
        retVal = false;
    }
    return retVal;
}


/*
 * Initialize the Sled Support serial port hardware.
 */
bool SledSupport::init( void )
{
    qDebug() << "Initializing FTDI device";
    bool status = true;
    unsigned long ftdiDeviceCount = 0;

    newMode = -1;
    newOffset = -1;
    newGain = -1;
    newSpeed = -1;
    newDevice = -1;
    newDir = -1;

    ftStatus = FT_CreateDeviceInfoList(&ftdiDeviceCount);
    if( ftdiDeviceCount == 0 )
    {
        qDebug() << "No FTDI Devices !!!";
    }
    else
    {
        qDebug() << "Number of FTDI Devices" << ftdiDeviceCount;
        ftdiDeviceInfo = (FT_DEVICE_LIST_INFO_NODE*) malloc (sizeof(FT_DEVICE_LIST_INFO_NODE)*ftdiDeviceCount);
        ftStatus = FT_GetDeviceInfoList( ftdiDeviceInfo, &ftdiDeviceCount );

        if( ftStatus == FT_OK )
        {
            for( unsigned int i = 0; i < ftdiDeviceCount; i++)
            {
                qDebug() << "*** FTDI Device number : " << i;
                qDebug() << "*** Flags    : " << ftdiDeviceInfo[i].Flags;
                qDebug() << "*** Type     : " << ftdiDeviceInfo[i].Type;
                qDebug() << "*** ID       : " << ftdiDeviceInfo[i].ID;
                qDebug() << "*** LocID    : " << ftdiDeviceInfo[i].LocId;
                qDebug() << "*** S/N      : " << ftdiDeviceInfo[i].SerialNumber;
                qDebug() << "*** Descript : " << ftdiDeviceInfo[i].Description;
                qDebug() << "*** ftHandle : " << ftdiDeviceInfo[i].ftHandle;
            }
        }
        ftHandle = ftdiDeviceInfo[0].ftHandle;
        // make sure device is closed before we open it
        ftStatus = FT_Close( ftHandle );
        ftStatus = FT_Open( 0, &ftHandle );
        if( ftStatus != FT_OK )
        {
            qDebug() << "Could not open FTDI device";
        }
        if( (strcmp(ftdiDeviceInfo[0].SerialNumber, "MS2816") == 0 ))
        {
            // This code is used to initialize the original L300 interface board (MS2816)
            QString msg = QString( "*** Prototype IF");
            qDebug() << msg;
//            LOG1(msg);
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xF4, 0x20 );   // SSB 5V on
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xF6, 0x20 );   // Sled 24V on
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xF7, 0x20 );   // Sled 5V on
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xFF, 0x20 );   // Laser on
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
        }
        else
        {
            // This code is used to initialize the final L300IF board (MS2916)
            QString msg = QString( "*** Final IF");
            qDebug() << msg;
//            LOG1(msg);
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xF4, 0x20 );  // First reset the board
            msg = QString("First reset the board");
//            LOG3(ftStatus, FT_OK, msg);
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xF2, 0x20 );   // Sled 24V on
            msg = QString("Sled 24V on");
//            LOG3(ftStatus, FT_OK, msg);
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xF3, 0x20 );   // Sled 5V on
            msg = QString("Sled 5V on");
//            LOG3(ftStatus, FT_OK, msg);
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
            ftStatus = FT_SetBitMode( ftHandle, 0xFB, 0x20 );   // Laser on
            msg = QString("Laser on");
//            LOG3(ftStatus, FT_OK, msg);
            if( ftStatus != FT_OK )
            {
                qDebug() << "Could not change bits";
            }
            Sleep(100);
        }

        ftStatus = FT_SetBaudRate( ftHandle, 9600);
        if( ftStatus != FT_OK )
        {
            qDebug() << "Could not set baud rate";
            LOG1(ftStatus);
        }
        ftStatus = FT_SetDataCharacteristics( ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
        if( ftStatus != FT_OK )
        {
            qDebug() << "Could not set data charecteristics";
            LOG1(ftStatus);
        }
        ftStatus = FT_SetFlowControl( ftHandle, FT_FLOW_NONE, 0x11, 0x13 );
        if( ftStatus != FT_OK )
        {
            qDebug() << "Could not set flow control";
            LOG1(ftStatus);
        }
        ftStatus = FT_SetTimeouts( ftHandle, 50, 1000 );
        if( ftStatus != FT_OK )
        {
            qDebug() << "Could not set timeouts ";
            LOG1(ftStatus);
        }
        ftStatus = FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
        if( ftStatus != FT_OK )
        {
            qDebug() << "Purge failed";
            LOG1(ftStatus);
        }
//        start();            // get run thread going
    }
    LOG1(status);
    return status;
}

/*
 * Signal to subscribers the updated clocking mode.
 */
void SledSupport::updateClockingMode( void )
{
    //qDebug() << "* SledSupport - clocking modes: " << prevClockingMode << " , " << currClockingMode;
    if( currClockingMode != prevClockingMode )
    {
        emit announceClockingMode( currClockingMode );
        prevClockingMode = currClockingMode;
        qDebug() << "Change in clocking mode. Mode:" << currClockingMode;
        LOG( INFO, QString( "Sled Support Board: Clocking mode change. Mode: %1" ).arg( currClockingMode ) );
    }
}

/*
 * handleClockingResponse
 */
void SledSupport::handleClockingResponse( void )
{
    prevSledState = currSledState;

    QByteArray resp = getResponse();

    if( resp != "" ) // test against an empty string
    {
        if( resp.toUpper().contains( "ACK" ) )
        {
            currSledState = SledSupport::ConnectedState; // assume connected if ACK

            int index = resp.toUpper().lastIndexOf( "GC" );
            if( index != -1 )
            {
                if( resp.toUpper().contains( "1" ) )  // gc=1 if clocking is on
                {
                    currClockingMode = SledSupport::NormalMode;
                }
                else if( resp.toUpper().contains( "0" ) ) // gc=0 if clocking is off
                {
                    currClockingMode = SledSupport::DiagnosticMode;
                }
                else  // there is no proper signature in the bytes returned
                {
                    currClockingMode = SledSupport::UnknownMode;
                    LOG( WARNING, QString( "Clocking mode is neither on nor off. Mode: %1, Response: %2" ).arg( currClockingMode ).arg( QString( resp ) ) );
                }
            }
        }
        else if (resp.toUpper().contains( "NAK") )
        {
            currClockingMode = SledSupport::UnknownMode;
            currSledState = SledSupport::DisconnectedState; // assume disconnected if NAK
        }
        else // Something other than ACK or NAK or empty string.
        {
            currSledState = SledSupport::UnknownState; // assume it is disconnected
            qDebug() << "Unknown response: " << resp;
            LOG( WARNING, QString( "Unknown response from Serial port for clocking mode: %1" ).arg( QString( resp ) ) );
        }
    }

    updateClockingMode();
}

/*
 * run
 *
 * This is the only code running in the new thread. It periodically loops around
 * to poll the Sled Support Board for clocking mode. If a re-connect state is
 * detected, it will push new clocking parameters to the Sled Support Board.
 */
void SledSupport::run()

{
    LOG1( "Start Sled polling")
    newDevice = 0;                                  // get started with device 0
    int pollingTimer = 0;
    isRunning = true;
    while( isRunning )
    {
        msleep( 10 );                               // 10 mSec between polling
        pollingTimer += 10;
        if( newMode >= 0 )
        {
            bool isEnabled = true;
            mutex.lock();
            if( newMode == 0 )
            {
                isEnabled = false;
            }
            newMode = -1;
            mutex.unlock();
            setClockingMode(isEnabled );
        }
        else if( newOffset > 0 )
        {
            mutex.lock();
            baParam.setNum( newOffset );
            newOffset = -1;
            mutex.unlock();
            setClockingOffset( baParam );
        }
        else if( newGain > 0 )
        {
            mutex.lock();
            baParam.setNum( newGain );
            newGain = -1;
            mutex.unlock();
            setClockingGain( baParam );
        }
        else if( newSpeed > 0 )
        {
            mutex.lock();
            LOG1(newSpeed)
            baParam.setNum( newSpeed );
            newSpeed = -1;
            mutex.unlock();
            setSledSpeed( baParam );
        }
        else if( newDevice >= 0 )
        {
            mutex.lock();
            deviceSettings &device = deviceSettings::Instance();
            LOG( INFO, QString( "Sled Support Board new Device Name: %1" ).arg( device.current()->getDeviceName() ) );
            int isEnabled = device.current()->getClockingEnabled();
            QByteArray clockingGain = device.current()->getClockingGain();
            QByteArray clockingOffset = device.current()->getClockingOffset();
            int speed = device.current()->getRevolutionsPerMin();
            LOG1(speed)
            QByteArray torqueLimit = device.current()->getTorqueLimit();
            QByteArray timeLimit = device.current()->getTimeLimit();
            mutex.unlock();
            newDevice = -1;

            emit setSlider( speed );
            baParam.setNum( speed );
            setSledSpeed( baParam );
            setClockingMode(isEnabled );
            setClockingOffset( clockingOffset );
            setClockingGain( clockingGain );
            setSledTorqueLimit( torqueLimit );
            setSledTimeLimit( timeLimit );

            enableBidirectional();
        }
        else if( pollingTimer > ClockingUpdateTimer_ms )
        {
            qDebug() << "****** Polling";
            pollingTimer = 0;
            mutex.lock();

            ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );   // flush input buffer
            if( ftStatus != FT_OK )
            {
                qDebug() << "Input flush failed";
            }
            writeSerial( GetClockingMode);
            msleep( SledCommDelay_ms );                 // sleep to wait for a response

            handleClockingResponse();                   // parse the response and update mode
            mutex.unlock();

            // Get firmware version when state changes
            if( currSledState != prevSledState)
            {
                qDebug() << "Sled state changed";
                getFirmwareVersions(); // Read Sled and Sled Support firmware versions and announce to advanced view.
            }

            // Reinstate clocking parameters when a state change from Disconnected to Connected is detected.
            if( prevSledState == SledSupport::DisconnectedState && currSledState == SledSupport::ConnectedState )
            {
                qDebug() << "Sled connected";
                setClockingParams( sledParams );
            }
        }
    }
}

/*
 *
 */
void SledSupport::setSpeed( int speed )
{
    newSpeed = speed;
}

void SledSupport::setMode( int mode )
{
    newMode = mode;
}
void SledSupport::setOffset( int offset )
{
    newOffset = offset;
}
void SledSupport::setGain( int gain )
{
    newGain = gain;
}
void SledSupport::setDevice( int deviceIndex)
{
    newDevice = deviceIndex;
    //qDebug() << "* SledSupport - new device: " << newDevice;
}
void SledSupport::setDirection( int dir )
{
    newDir = dir;
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

void SledSupport::setClockingParams( DeviceClockingParams_T params )
{
    qDebug() << "set clocking Params";
        setClockingMode( params.isEnabled );
        setClockingGain( params.gain );
        setClockingOffset( params.offset );
        setSledSpeed( params.speed );
        setSledTorqueLimit( params.torque );
        setSledTimeLimit( params.time );
        setSledLimitBlink( params.blinkEnabled );
}

/*
 * updateDeviceForSledSupport
 *
 * This slot is executed in main and signaled by the gui. Call to set up
 * parameters for a new device. First completes run() by calling stop(),
 * then shared resources can be assumed safe for use.
 */
/*
void SledSupport::updateDeviceForSledSupport( bool isEnabled, QByteArray clockingGain, QByteArray clockingOffset, int speed, QByteArray torque, QByteArray time )
{
    if( isRunning )
    {
        stop(); // blocks until run() completes
    }
    qDebug() << "* SledSupport - Update Device for Sled Support";

    sledParams.isEnabled   = isEnabled;
    sledParams.gain        = clockingGain;
    sledParams.offset      = clockingOffset;
    sledParams.speed.setNum( speed );
    emit setSlider( speed );
    sledParams.torque      = torque;
    sledParams.time        = time;

    getFirmwareVersions(); // get versions from SSB and send to advanced view
    //qDebug() << "Sled version; " << sledParams.vSled << "Sled Support version: " << sledParams.vSSB;

    setClockingParams( sledParams );

    //start();
}
*/
/*
 * Send command to set clocking correction ON/OFF.
 */
void SledSupport::setClockingMode( int mode )
{
    QByteArray compareVal;

    emit setCorrection( mode );

    if( ftHandle != NULL )
    {
        mutex.lock();

        compareVal = QByteArray( "gc=" ).append( QString::number( mode ) );
        QByteArray setClockingSerialCmd = QByteArray( SetClockingMode ).append( QString::number( mode ) ).append( "\r" );
//        qDebug() << "Tx:" << setClockingSerialCmd;

        writeSerial( setClockingSerialCmd );

        //LOG( INFO, QString( "Sled Support Board: set clocking mode: %1" ).arg( QString::number( mode ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        //qDebug() << "Rx:" << resp;

        if( resp.toUpper().contains( "NAK" ) )
        {
            qDebug() << "set clocking returned NAK" << resp.toUpper();
            //LOG( WARNING, QString( "Sled Support Board: set clocking returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }

        // Confirm the Set value matches the Get value.
        mutex.lock();
        writeSerial( GetClockingMode );
        Sleep( SledCommDelay_ms );
        resp = getResponse();
        mutex.unlock();

        // Report the results for DEBUG purposes.
        if( resp.contains( compareVal ) )
        {
            qDebug() << "setClockingMode SUCCESS - " << resp;
        }
        else
        {
            qDebug() << "setClockingMode FAILED: " << QString( resp );
        }
    }
}

/*
 * Send command to set clocking gain.
 */
void SledSupport::setClockingGain( QByteArray gain )
{
    emit setGainSlider( gain.toInt() );

    // remember gain in case Sled of off-line
    sledParams.gain =  gain ;

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
            qDebug() << "set clocking gain returned NAK" << resp.toUpper();
            //LOG( WARNING, QString( "Sled Support Board: set clocking gain returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }

        // Confirm the Set value matches the Get value.
        mutex.lock();
        writeSerial( GetClockingGain );
        Sleep( SledCommDelay_ms );
        resp = getResponse();
        mutex.unlock();

        // Report the results for DEBUG purposes.
        if( resp.contains( gain ) )
        {
            qDebug() << "setClockingGain SUCCESS - " << resp;
        }
        else
        {
            qDebug() << "setClockingGain FAILED: " << QString( resp );
        }
    }
}

/*
 * Send command to set clocking offset.
 */
void SledSupport::setClockingOffset( QByteArray offset )
{
    emit setOffsetSlider( offset.toInt() );

    // remember offset in case Sled of off-line
    sledParams.offset =  offset ;

    QByteArray setOffsetSerialCmd = QByteArray( SetClockingOffset ).append( offset ).append( "\r" );

    if( ftHandle != NULL )
    {
        mutex.lock();

        //qDebug() << "Tx:" << setOffsetSerialCmd;
        writeSerial( setOffsetSerialCmd );
        //LOG( INFO, QString( "Sled Support Board: set clocking offset: %1" ).arg( QString( offset ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        //qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
            qDebug() << "set clocking offset returned NAK" << resp.toUpper();
            //LOG( WARNING, QString( "Sled Support Board: set clocking offset returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }

        // Confirm the Set value matches the Get value.
        mutex.lock();
        writeSerial( GetClockingOffset );
        Sleep( SledCommDelay_ms );
        resp = getResponse();
        mutex.unlock();

        // Report the results for DEBUG purposes.
        if( resp.contains( offset ) )
        {
            qDebug() << "setClockingOffset SUCCESS - " << resp;
        }
        else
        {
            qDebug() << "setClockingOffset FAILED: " << QString( resp );
        }
    }
}

/*
 * setSledSpeed
 */

void SledSupport::setSledSpeed( QByteArray speed )
{
    if( ftHandle != NULL )
    {
        announceSpeed( speed );
        // remember speed in case Sled of off-line
        sledParams.speed =  speed ;

        QByteArray setSpeedSerialCmd = QByteArray( SetSpeed ).append( speed ).append( "\r" );

        mutex.lock();

        //qDebug() << "Tx:" << setSpeedSerialCmd;
        writeSerial( setSpeedSerialCmd );
        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        //qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
            qDebug() << "set speed returned NAK" << resp.toUpper();
            //LOG( WARNING, QString( "Sled Support Board: set speed returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }

        // Confirm the Set value matches the Get value.
        mutex.lock();
        writeSerial( GetSpeed );
        Sleep( SledCommDelay_ms );
        resp = getResponse();
        mutex.unlock();

        // Report the results for DEBUG purposes.
        if( resp.contains( speed ) )
        {
            qDebug() << "setSledSpeed SUCCESS - " << resp;
        }
        else
        {
            qDebug() << "setSledSpeed FAILED: " << QString( resp );
        }
    }
}

/*
 * setVOA
 */

void SledSupport::setPower( int milliWattTenth )
{
    if( ftHandle != NULL )
    {
        QByteArray setVOASerialCmd = QByteArray( SetPower ).append( QString::number( milliWattTenth )).append( "\r" );

        mutex.lock();

        qDebug() << "Tx:" << setVOASerialCmd;
        writeSerial( setVOASerialCmd );
        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        if( resp.toUpper().contains( "NAK" ) )
        {
            qDebug() << "set speed returned NAK" << resp.toUpper();
            //LOG( WARNING, QString( "Sled Support Board: set speed returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }
    }
}

bool SledSupport::isRunningState()
{
    bool running = false;
    if( ftHandle != NULL )
    {
        // first get current run mode
        mutex.lock();
        ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );   // flush input buffer
        if( ftStatus != FT_OK )
        {
            qDebug() << "Input flush failed";
        }
        writeSerial( GetRunningState );
        msleep( SledCommDelay_ms );                 // sleep to wait for a response
        QByteArray resp = getResponse();
        mutex.unlock();
        qDebug() << "get running state response:" << resp;
        if( resp.toUpper().contains( "1" )) {
            running = true;
        }
        //1015 is UTF-16, 1014 UTF-16LE, 1013 UTF-16BE, 106 UTF-8
        QString respAsString = QTextCodec::codecForMib(106)->toUnicode(resp);
//        LOG2(respAsString, running)
    }
    return running;
}

int SledSupport::runningState()
{
    int running = -1;
    if( ftHandle != NULL )
    {
        // first get current run mode
        mutex.lock();
        ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );   // flush input buffer
        if( ftStatus != FT_OK )
        {
            qDebug() << "Input flush failed";
        }
        writeSerial( GetRunningState );
        msleep( SledCommDelay_ms );                 // sleep to wait for a response
        QByteArray resp = getResponse();
        mutex.unlock();
        qDebug() << "get running state response:" << resp;
        if( resp.toUpper().contains( "1" )) {
            running = 1;
        } else if(resp.toUpper().contains( "3" )){
            running = 3;
        }
        //1015 is UTF-16, 1014 UTF-16LE, 1013 UTF-16BE, 106 UTF-8
        QString respAsString = QTextCodec::codecForMib(106)->toUnicode(resp);
//        LOG2(respAsString, running)
    }
    return running;
}


/*
 * setSledTorque
 */

void SledSupport::setSledTorqueLimit(QByteArray torque)
{
    int temp = (int) (torque.toFloat() * 10);   // Sled expects parameter without the decimal point
    torque.setNum( temp );
    QByteArray setTorqueSerialCmd = QByteArray( SetTorque ).append( torque ).append( "\r" );
    if( ftHandle != NULL )
    {
        sledParams.torque = torque;
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
            qDebug() << "set torque returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set torque returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }

        // Confirm the Set value matches the Get value.
        mutex.lock();
        writeSerial( GetTorque );
        Sleep( SledCommDelay_ms );
        resp = getResponse();
        mutex.unlock();

        // Report the results for DEBUG purposes.
        if( resp.contains( torque ) )
        {
            qDebug() << "setTorque SUCCESS - " << resp;
        }
        else
        {
            qDebug() << "setTorque FAILED: " << QString( resp );
        }
    }
}

/*
 * setSledLimitTime
 */

void SledSupport::setSledTimeLimit(QByteArray limit)
{
    QByteArray setLimitSerialCmd = QByteArray( SetLimitTime ).append( limit ).append( "\r" );
    if( ftHandle != NULL )
    {
        sledParams.time = limit;
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
            qDebug() << "set limit returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set limit returned NAK. Response: %1" ).arg( QString( resp ) ) );
        }

        // Confirm the Set value matches the Get value.
        mutex.lock();
        writeSerial( GetLimitTime );
        Sleep( SledCommDelay_ms );
        resp = getResponse();
        mutex.unlock();

        // Report the results for DEBUG purposes.
        if( resp.contains( limit ) )
        {
            qDebug() << "setLimitTime SUCCESS - " << resp;
        }
        else
        {
            qDebug() << "setLimitTime FAILED: " << QString( resp );
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

        qDebug() << "Tx:" << setBlinkSerialCmd;
        writeSerial( setBlinkSerialCmd );
        LOG( INFO, QString( "Sled Support Board: set blink: %1" ).arg( char( isBlinkEnabled ) ) );

        Sleep( SledCommDelay_ms );
        QByteArray resp = getResponse();
        mutex.unlock();

        qDebug() << "Rx:" << resp;
        if( resp.toUpper().contains( "NAK" ) )
        {
            qDebug() << "set blink returned NAK" << resp.toUpper();
            LOG( WARNING, QString( "Sled Support Board: set blink returned NAK. Response: %1" ).arg( QString( resp ) ) );
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
    if( ftHandle != NULL )
    {
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
         * display "N/A" if not. The version strings are messaged to the gui
         */

        qDebug() << "***** version: " << resp;
        if( resp.startsWith( strPrefix.toLatin1() ) )
        {
            // parse the response
            resp.remove( 0, strPrefix.length() ); // remove 7 characters "ACK gv="

            sledVersion = qualifyVersion( resp.left( versionTemplate.length() ) );
            ssbVersion  = qualifyVersion( resp.right( versionTemplate.length() ) );
        }
        sledParams.vSled = sledVersion;
        sledParams.vSSB  = ssbVersion;
        qDebug() << "versions: " << sledVersion << " , " << ssbVersion;
        emit announceFirmwareVersions( sledVersion, ssbVersion );
        //LOG( INFO, QString( "Firmware versions: Sled - %1, Sled Support Board - %2" ).arg( QString( sledVersion ) ).arg( QString( ssbVersion ) ) );

        if( sledParams.vSled.startsWith( "1.") )
        {
            qDebug() << "Sled version 1.X is not supported";
            // handle error
        }
        if( sledParams.vSSB.startsWith( "1.") )
        {
            qDebug() << "Sled Support Board version 1.X, is not supported";
            // handle error
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

QString SledSupport::commandToString(const QByteArray &ba)
{
    QString cmd(ba.simplified());

    const auto it = commandLut.find(cmd);

    QString fcmd;

    if(it != commandLut.end()){
        fcmd = QString("code: \"") + cmd + QString("\" verbose: ") + it->second;
    } else {
        fcmd = QString("code: \"") + cmd + QString("\"");
    }
    return fcmd;
}

/*
 * Read available data from the serial port.
 */

QByteArray SledSupport::getResponse( void )
{
    QByteArray data;
    DWORD bytesToRead = 256;
    DWORD bytesRead;
    char buffer[256]= {};

    //qDebug() << "Reading Sled Response";

    ftStatus = FT_Read( ftHandle, buffer, bytesToRead, &bytesRead );
    if( ftStatus != FT_OK )
    {
        qDebug() << "Serial read failed";
    }
    else
    {
        buffer[bytesRead] = '\0';
        data = buffer;
        data = data.simplified();
    }
    if(data.toUpper().contains( "NAK" )){
        LOG( INFO, QString("Sled Support getResponse data: %1").arg(bytesRead).arg(commandToString(buffer)) );
    }
    return data;
}

void SledSupport::enableBidirectional()
{
    deviceSettings &device = deviceSettings::Instance();

    auto currentDev = device.current();
    const bool isBiDirectionalEnabled{currentDev->isBiDirectional()};
    if(isBiDirectionalEnabled){
        writeSerial("sbm1\r");
    } else {
         writeSerial("sbm0\r");
    }
}


