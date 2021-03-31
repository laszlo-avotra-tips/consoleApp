#include "interfacesupport.h"
#include <QDebug>
#include "logger.h"

constexpr auto DEVICE_BIT_POSITION_SLED_5V = 0;
constexpr auto DEVICE_BIT_POSITION_SLED_24V = 1;
constexpr auto DEVICE_BIT_POSITION_INTERFACE_BOARD_RESET = 2;
constexpr auto DEVICE_BIT_POSITION_LASER = 3;

InterfaceSupport* InterfaceSupport::m_instance {nullptr};

InterfaceSupport *InterfaceSupport::getInstance(bool resetInterfaceBoard) {
    if(!m_instance){
        m_instance = new InterfaceSupport();
        // Initialize FTDI device
        if (!m_instance->initalizeFTDIDevice()) {
            LOG(ERROR, "Failed to initialize InterfaceSupport device");
            delete m_instance;
            m_instance = nullptr;
        } else {
            if (resetInterfaceBoard) {
                // Now perform one time reset on the interface board to start using interface support
                if (!m_instance->resetInterfaceBoard()) {
                    LOG(ERROR, "Failed to perform reset on the interface board")
                } else {
                    LOG(INFO, "Interface board successfully reset")
                }
            }
        }
    }

    return m_instance;
}

void InterfaceSupport::releaseInstance() {
    if(m_instance){
        delete m_instance;
        m_instance = nullptr;
    }
}

InterfaceSupport::~InterfaceSupport() {
    FT_STATUS ftStatus = FT_Close(ftHandle);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not close FTDI device";
    }

    if (ftdiDeviceInfo) {
        delete [] ftdiDeviceInfo;
        ftdiDeviceInfo = nullptr;
        ftHandle = nullptr;
    }
}

void InterfaceSupport::prepDevice() {
    if (ftHandle == nullptr) {
        qDebug() << "FTDI device not initialized yet!";
        return;
    }

    FT_STATUS ftStatus = FT_SetBaudRate( ftHandle, 9600);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not set baud rate";
    }
    ftStatus = FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
    if( ftStatus != FT_OK ) {
        qDebug() << "Purge failed";
    }
    ftStatus = FT_SetDataCharacteristics( ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not set data charecteristics";
    }
    ftStatus = FT_SetFlowControl( ftHandle, FT_FLOW_NONE, 0x11, 0x13 );
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not set flow control";
    }
    ftStatus = FT_SetTimeouts( ftHandle, 50, 1000 );
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not set timeouts ";
    }
}

bool InterfaceSupport::initalizeFTDIDevice() {
    LOG( INFO, "Initializing FTDI device");
    unsigned long ftdiDeviceCount = 0;

    FT_STATUS ftStatus = 0;
    ftStatus = FT_CreateDeviceInfoList(&ftdiDeviceCount);
    if( ftdiDeviceCount == 0 ) {
        qDebug() << "No FTDI Devices !!!";
        LOG( ERROR, "No FTDI Devices found!!!");
        return false;
    } else {
        LOG( INFO, QString("Number of FTDI Devices : %1").arg(ftdiDeviceCount));
        ftdiDeviceInfo = new FT_DEVICE_LIST_INFO_NODE[ftdiDeviceCount];
        ftStatus = FT_GetDeviceInfoList( ftdiDeviceInfo, &ftdiDeviceCount );

        if( ftStatus == FT_OK ) {
            for( unsigned int i = 0; i < ftdiDeviceCount; i++) {
                LOG( INFO, QString("*** FTDI Device number : %1").arg(i));
                LOG( INFO, QString("*** Flags : %1").arg(ftdiDeviceInfo[i].Flags));
                LOG( INFO, QString("*** Type : %1").arg(ftdiDeviceInfo[i].Type));
                LOG( INFO, QString("*** ID : %1").arg(ftdiDeviceInfo[i].ID));
                LOG( INFO, QString("*** LocID : %1").arg(ftdiDeviceInfo[i].LocId));
                LOG( INFO, QString("*** S/N : %1").arg(ftdiDeviceInfo[i].SerialNumber));
                LOG( INFO, QString("*** Description : %1").arg(ftdiDeviceInfo[i].Description));
            }
        }

        ftHandle = ftdiDeviceInfo[0].ftHandle;

        // make sure device is closed before we open it
        ftStatus = FT_Close( ftHandle );
        ftStatus = FT_Open( 0, &ftHandle );
        if( ftStatus != FT_OK ) {
            qDebug() << "Could not open FTDI device";
            return false;
        }

        UCHAR currentVal;
        ftStatus = FT_GetBitMode(ftHandle, &currentVal);
        if( ftStatus != FT_OK ) {
            qDebug() << "Could not read current FTDI mode";
            return false;
        }

        prepDevice();

        currentBitSetVal = currentVal;
        populateInterfaceBoardCommandList();

        return true;
    }
}

bool InterfaceSupport::resetInterfaceBoard() {
    bool result = true;

    QString msg = "Reset interface board";
    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, 0xF0, 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not perform reset on interface board" << msg;
        return false;
    }

    turnOnACPowerToOCT(false);
    return result;
}

bool InterfaceSupport::turnOffInterfaceBoard() {
    return performResetLow();
}

bool InterfaceSupport::turnOnSled5V(bool state) {
    currentBitSetVal.set(DEVICE_BIT_POSITION_SLED_5V, state);

    QString msg = "";

    if (state == true) {
        msg = "turn on Sled 5V";
    } else {
        msg = "turn off Sled 5V";
    }

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not complete operation for Sled 5V" << msg;
        return false;
    }

    return true;
}

bool InterfaceSupport::turnOnSled24V(bool state) {
    currentBitSetVal.set(DEVICE_BIT_POSITION_SLED_24V, state);

    QString msg = "";

    if (state == true) {
        msg = "turn on Sled 24V";
    } else {
        msg = "turn off Sled 24V";
    }

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not complete operation for Sled 24V" << msg;
        return false;
    }

    return true;
}

bool InterfaceSupport::turnOnSled(bool state) {
    bool result = turnOnSled5V(state);

    if (result) {
        turnOnSled24V(state);
    }

    return result;
}

bool InterfaceSupport::turnOnLaser(bool state) {
    currentBitSetVal.set(DEVICE_BIT_POSITION_LASER, state);

    QString msg = "";

    if (state == true) {
        msg = "turn on Laser";
    } else {
        msg = "turn off Laser";
    }

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not complete operation for Laser" << msg;
        return false;
    }

    return true;
}

bool InterfaceSupport::performResetLow() {
    currentBitSetVal.set(DEVICE_BIT_POSITION_INTERFACE_BOARD_RESET, true);

    QString msg = "perform reset low";

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not complete operation for low resetting of interface board" << msg;
        return false;
    }

    return true;
}

bool InterfaceSupport::performResetHigh() {
    currentBitSetVal.set(DEVICE_BIT_POSITION_INTERFACE_BOARD_RESET, false);

    QString msg = "perform reset high";

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not complete operation for high resetting of interface board" << msg;
        return false;
    }

    return true;
}

bool InterfaceSupport::reportCurrentDeviceStatus() {

    UCHAR currentVal;
    FT_STATUS ftStatus = FT_GetBitMode(ftHandle, &currentVal);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not read current FTDI mode";
        return false;
    }

    std::bitset<8> currBitSetVal = currentVal;

    float hardwareVersion = getHardwareVersion();
    qDebug() << "H/W version:" << hardwareVersion;
    float firmwareVersion = getFirmwareVersion();
    qDebug() << "F/W version:" << firmwareVersion;
    float sledVersion = getSledFirmwareVersion();

    if (sledVersion == 0) {
        qDebug() << "Sled F/W   :" << "xx.xx";
    } else {
        qDebug() << "Sled F/W   :" << sledVersion;
    }

    qDebug() << "VOA        :" << getVOASettings();

    if (currBitSetVal.test(DEVICE_BIT_POSITION_INTERFACE_BOARD_RESET)) {
        qDebug() << "Low reset:   ON";
    } else {
        qDebug() << "High reset:  ON";
    }

    if (currBitSetVal.test(DEVICE_BIT_POSITION_SLED_5V)) {
        qDebug() << "Sled 5V:     ON";
    } else {
        qDebug() << "Sled 5V:     OFF";
    }

    if (currBitSetVal.test(DEVICE_BIT_POSITION_SLED_24V)) {
        qDebug() << "Sled 24V:    ON";
    } else {
        qDebug() << "Sled 24V:    OFF";
    }

    if (currBitSetVal.test(DEVICE_BIT_POSITION_LASER)) {
        qDebug() << "Laser:       ON";
    } else {
        qDebug() << "Laser:       OFF";
    }

    float minimumACVoltage = (float) 122 * 0.9f;
    float detectedACVoltage = getSupplyVoltage();

    if (minimumACVoltage > detectedACVoltage) {
        qDebug() << "AC Power:    OFF";
    } else {
        qDebug() << "AC Power:    ON";
    }

    int runState = getRunningState();
    QString runDesc = "SLED OFF";
    switch (runState) {
    case 0:
        runDesc = "STOPPED";
        break;
    case 1:
        runDesc = "CLOCKWISE";
        break;
    case 3:
        runDesc = "COUNTER-CLOCKWISE";
        break;
    default:
        break;
    }

    qDebug() << "Sled state: " << runDesc;

    return true;
}

QByteArray InterfaceSupport::readDataFromDevice(bool ignoreNakResponse) {
    QByteArray data;
    if(ftHandle != nullptr) {
        DWORD bytesToRead = 256;
        DWORD bytesRead;
        char buffer[256]= {};

        Sleep(100);
        FT_STATUS ftStatus = FT_Read( ftHandle, buffer, bytesToRead, &bytesRead );
        if( ftStatus != FT_OK ) {
            qDebug() << "Serial read failed";
        } else {
            buffer[bytesRead] = '\0';
            data = buffer;
            data = data.simplified();
        }

        if(!ignoreNakResponse && data.toUpper().contains( "NAK" )){
            LOG( INFO, "Device responded NAK");
        }
    }
    return data;
}

bool InterfaceSupport::writeDataToDevice(QByteArray command) {
    bool retVal = true;

    if( ftHandle != NULL ) {
        DWORD bytesWritten;

        FT_STATUS ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );   // flush input buffer
        if( ftStatus != FT_OK) {
            qDebug() << "Input flush failed";
        }

        Sleep(100);
        ftStatus = FT_Write( ftHandle, command.data(), command.size(), &bytesWritten);
        if( ftStatus != FT_OK ) {
            qDebug() << "Could not write command" << command;
            LOG( WARNING, QString( "Interface support: writeDataToDevice could not write command: %1 ").arg(command.data()));
            retVal = false;
        } else {
            //qDebug() << "Serial bytes written: " << bytesWritten;
            LOG( INFO, QString( "Interface support: writeDataToDevice command written: %1 " ).arg( command.data() ) );
        }
    } else {
        qDebug() << "Serial port not open for write";
        retVal = false;
    }

    return retVal;
}

void InterfaceSupport::populateInterfaceBoardCommandList() {
    interfaceBoardCommandList.clear();

    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::GET_HARDWARE_VERSION, "ghv\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::GET_FIRMWARE_VERSION, "gfv\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::GET_SUPPLY_VOLTAGE,   "gsv\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_AC_POWER_OFF_TO_OCT_ENGINE, "sac0\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_AC_POWER_ON_TO_OCT_ENGINE, "sac1\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::GET_VOA_SETTINGS,   "gvo\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_VOA_TO_TRANSPARENT_MODE, "svt\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_VOA_TO_BLOCKING_MODE, "svb\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::GET_SLED_FIRMWARE_VERSION, "gv\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_SLED_SPEED, "ss");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_BIDIRECTIONAL_MODE_OFF, "sbm0\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_BIDIRECTIONAL_MODE_ON, "sbm1\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::GET_SLED_RUNNING_STATE, "gr\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::STOP_SLED, "sr0\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::START_SLED, "sr1\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_COUNTER_CLOCKWISE_DIRECTION, "sd0\r");
    interfaceBoardCommandList.emplace(OctInterfaceBoardCommandType::SET_CLOCKWISE_DIRECTION, "sd1\r");
}

float InterfaceSupport::getHardwareVersion() {
    float hardwareVersionNum = 0.0;

    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_HARDWARE_VERSION])) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            for (auto part : parts) {
                if (part.contains("ghv")) {
                    auto version = part.split(QLatin1Char('='));
                    hardwareVersionNum = version[1].toFloat();
                    LOG( INFO, QString( "Interface support: getHardwareVersion response: %1 ").arg(hardwareVersionNum));
                    break;
                }
            }
        }
    }

    return hardwareVersionNum;
}

float InterfaceSupport::getFirmwareVersion() {
    float firmwareVersionNum = 0.0;

    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_FIRMWARE_VERSION])) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            for (auto part : parts) {
                if (part.contains("gfv")) {
                    auto version = part.split(QLatin1Char('='));
                    firmwareVersionNum = version[1].toFloat();
                    LOG( INFO, QString( "Interface support: getFirmwareVersion response: %1 ").arg(firmwareVersionNum));
                    break;
                }
            }
        }
    }

    return firmwareVersionNum;
}

float InterfaceSupport::getSupplyVoltage() {
    float supplyVoltageVal = 0.0;

    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_SUPPLY_VOLTAGE])) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            auto version = parts[0].split(QLatin1Char('='));
            supplyVoltageVal = version[1].toFloat();
            LOG( INFO, QString( "Interface support: getSupplyVoltage response: %1 ").arg(supplyVoltageVal));
        }
    }

    return supplyVoltageVal;
}

bool InterfaceSupport::turnOnSledAndLaser() {
    bool result = true;

    QString msg = "Turn on Sled and Laser";
    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, 0xFB, 0x20 );  // Turn on Sled and laser
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qDebug() << "Could not turn on Sled and Laser" << msg;
        return false;
    }

    return result;
}

float InterfaceSupport::getSledFirmwareVersion() {
    float sledFirmwareVersion = 0.0;

    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_SLED_FIRMWARE_VERSION])) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            auto combinedVersion = parts[0].split(QLatin1Char('='));
            auto versionList = combinedVersion[1].split(QLatin1Char('_'));

            if (versionList[0] == "xx.xx") {
                LOG( ERROR, "Sled power is turned off. Could not obtain Sled firmware version.");
                return sledFirmwareVersion;
            }

            sledFirmwareVersion = versionList[0].toFloat();
            LOG( INFO, QString( "Interface support: getSledFirmwareVersion response: %1 ").arg(sledFirmwareVersion));
        }
    }

    return sledFirmwareVersion;
}

bool InterfaceSupport::turnOnACPowerToOCT(bool state) {
    QByteArray command;

    if (state) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_AC_POWER_ON_TO_OCT_ENGINE];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_AC_POWER_OFF_TO_OCT_ENGINE];
    }

    bool operationResult = false;

    if (writeDataToDevice(command)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;

            if (state) {
                LOG( INFO, "Successfully turned on AC power to OCT");
            } else {
                LOG( INFO, "Successfully turned off AC power to OCT");
            }
        } else {
            if (state) {
                LOG( ERROR, "Failed to turn on AC power to OCT");
            } else {
                LOG( ERROR, "Failed to turn off AC power to OCT");
            }
        }
    }

    return operationResult;
}

int InterfaceSupport::getVOASettings() {
    int voaValue = -1;

    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_VOA_SETTINGS])) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            for (auto part : parts) {
                if (part.contains("gvo")) {
                    auto version = part.split(QLatin1Char('='));
                    voaValue = version[1].toInt();
                    LOG( INFO, QString( "Interface support: getVOASettings response: %1 ").arg(voaValue));
                    break;
                }
            }
        }
    }

    return voaValue;
}

bool InterfaceSupport::setVOAMode(bool transparentMode) {
    QByteArray command;

    if (transparentMode) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_VOA_TO_TRANSPARENT_MODE];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_VOA_TO_BLOCKING_MODE];
    }

    bool operationResult = false;

    if (writeDataToDevice(command)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;

            if (transparentMode) {
                LOG( INFO, "Successfully set VOA to transparent mode");
            } else {
                LOG( INFO, "Successfully set VOA to blocking mode");
            }
        } else {
            if (transparentMode) {
                LOG( ERROR, "Failed to set VOA to transparent mode");
            } else {
                LOG( ERROR, "Failed to set VOA to blocking mode");
            }
        }
    }

    return operationResult;
}

bool InterfaceSupport::setSledSpeed(int speed) {
    QByteArray setSpeedSerialCmd = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_SLED_SPEED];
    const QString speedstr(QString::number(speed));
    const QByteArray baSpeed(speedstr.toStdString().c_str());

    setSpeedSerialCmd.append(baSpeed).append("\r");

    bool operationResult = false;

    if (writeDataToDevice(setSpeedSerialCmd)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;
            LOG( INFO, QString( "Successfully set speed to Sled: %1 ").arg(speed));
        } else {
            LOG( ERROR, QString( "Failed to set speed to Sled: %1 ").arg(speed));
        }
    }

    return operationResult;
}

bool InterfaceSupport::enableDisableBidirectional(bool bidirectionalState) {
    QByteArray command;

    if (bidirectionalState) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_BIDIRECTIONAL_MODE_ON];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_BIDIRECTIONAL_MODE_OFF];
    }

    bool operationResult = false;

    if (writeDataToDevice(command)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;

            if (bidirectionalState) {
                LOG( INFO, "Successfully turned on bidirectional state");
            } else {
                LOG( INFO, "Successfully turned off bidirectional state");
            }
        } else {
            if (bidirectionalState) {
                LOG( ERROR, "Failed to turn on bidirectional state");
            } else {
                LOG( ERROR, "Failed to turn off bidirectional state");
            }
        }
    }

    return operationResult;
}

int InterfaceSupport::getRunningState() {
    int runState = 0;

    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_SLED_RUNNING_STATE])) {
        bool ignoreNakResponse = true;
        QByteArray response = readDataFromDevice(ignoreNakResponse);
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            for (auto part : parts) {
                if (part.contains("gr")) {
                    auto version = part.split(QLatin1Char('='));
                    runState = version[1].toInt();
                    LOG( INFO, QString( "Interface support: getRunningState response: %1 ").arg(runState));
                    break;
                }
            }
        }
    }

    return runState;
}

bool InterfaceSupport::setSledRunState(bool runState) {
    QByteArray command;

    if (runState) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::START_SLED];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::STOP_SLED];
    }

    bool operationResult = false;

    if (writeDataToDevice(command)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;

            if (runState) {
                LOG( INFO, "Successfully started Sled");
            } else {
                LOG( INFO, "Successfully stopped Sled");
            }
        } else {
            if (runState) {
                LOG(ERROR, "Failed to start sled");
            } else {
                LOG(ERROR, "Failed to start sled");
            }
        }
    }

    return operationResult;
}

bool InterfaceSupport::setSledDirection(bool direction) {
    QByteArray command;

    if (direction) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_CLOCKWISE_DIRECTION];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_COUNTER_CLOCKWISE_DIRECTION];
    }

    bool operationResult = false;

    if (writeDataToDevice(command)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;

            if (direction) {
                LOG( INFO, "Successfully changed direction of Sled to clockwise");
            } else {
                LOG( INFO, "Successfully changed direction of Sled to counter-clockwise");
            }
        } else {
            if (direction) {
                LOG(ERROR, "Failed to change direction of Sled to clockwise");
            } else {
                LOG(ERROR, "Failed to change direction of Sled to counter-clockwise");
            }
        }
    }

    return operationResult;
}
