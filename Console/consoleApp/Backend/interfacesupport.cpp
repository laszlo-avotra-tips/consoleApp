#include "interfacesupport.h"
#include <QDebug>
#include <QTextStream>
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
        const QString msg( "Could not close FTDI device");
        LOG1(msg);
    }

    if (ftdiDeviceInfo) {
        delete [] ftdiDeviceInfo;
        ftdiDeviceInfo = nullptr;
        ftHandle = nullptr;
    }
}

void InterfaceSupport::prepDevice() {
    if (ftHandle == nullptr) {
        const QString msg("FTDI device not initialized yet!");
        LOG1(msg)
        return;
    }

    FT_STATUS ftStatus = FT_SetBaudRate( ftHandle, 9600);
    if( ftStatus != FT_OK ) {
        const QString msg("Could not set baud rate");
        LOG1(msg)
    }
    ftStatus = FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
    if( ftStatus != FT_OK ) {
        const QString msg("Purge failed");
        LOG1(msg)
    }
    ftStatus = FT_SetDataCharacteristics( ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    if( ftStatus != FT_OK ) {
        const QString msg("Could not set data charecteristics");
        LOG1(msg)
    }
    ftStatus = FT_SetFlowControl( ftHandle, FT_FLOW_NONE, 0x11, 0x13 );
    if( ftStatus != FT_OK ) {
        const QString msg("Could not set flow control");
        LOG1(msg)
    }
    ftStatus = FT_SetTimeouts( ftHandle, 50, 1000 );
    if( ftStatus != FT_OK ) {
        const QString msg("Could not set timeouts ");
        LOG1(msg)
    }
}

bool InterfaceSupport::initalizeFTDIDevice() {
    LOG( INFO, "Initializing FTDI device");
    unsigned long ftdiDeviceCount = 0;

    FT_STATUS ftStatus = 0;
    ftStatus = FT_CreateDeviceInfoList(&ftdiDeviceCount);
    if( ftdiDeviceCount == 0 ) {
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
            const QString msg("Could not open FTDI device");
            LOG1(msg)
            return false;
        }

        UCHAR currentVal;
        ftStatus = FT_GetBitMode(ftHandle, &currentVal);
        if( ftStatus != FT_OK ) {
            const QString msg("Could not read current FTDI mode");
            LOG1(msg)
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

    QString msg = "Reset interface board - pull reset line low";
    QTextStream qts(&msg);

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, 0xF4, 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qts << "Could not perform reset on interface board" << msg;
        LOG1(msg)
        return false;
    }

    msg = "Reset interface board - pull reset line high";
    ftStatus = FT_SetBitMode( ftHandle, 0xF0, 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qts << "Could not perform reset on interface board" << msg;
        LOG1(msg)
        return false;
    }

    turnOnOffACPowerToOCT(false);
    return result;
}

bool InterfaceSupport::turnOffInterfaceBoard() {
    return performResetLow();
}

bool InterfaceSupport::turnOnOffSled5V(bool isOn) {

    LOG1(isOn)
    currentBitSetVal.set(DEVICE_BIT_POSITION_SLED_5V, isOn);

    QString msg;
    QTextStream qts(&msg);

    if (isOn == true) {
        msg = "turn on Sled 5V";
    } else {
        msg = "turn off Sled 5V";
    }

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qts << "Could not complete operation for Sled 5V" << msg;
        LOG1(msg)
        return false;
    }

    return true;
}

bool InterfaceSupport::turnOnOffSled24V(bool isOn) {

    LOG1(isOn)

    currentBitSetVal.set(DEVICE_BIT_POSITION_SLED_24V, isOn);

    QString msg;
    QTextStream qts(&msg);

    if (isOn == true) {
        msg = "turn on Sled 24V";
    } else {
        msg = "turn off Sled 24V";
    }

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qts << "Could not complete operation for Sled 24V" << msg;
        LOG1(msg)
        return false;
    }

    return true;
}

bool InterfaceSupport::turnOnOffSled(bool isOn) {

    LOG1(isOn)
    bool result = turnOnOffSled5V(isOn);

    if (result) {
        turnOnOffSled24V(isOn);
    }

    return result;
}

bool InterfaceSupport::turnOnOffLaser(bool isOn) {

    LOG1(isOn)

    currentBitSetVal.set(DEVICE_BIT_POSITION_LASER, isOn);

    QString msg;
    QTextStream qts(&msg);

    if (isOn == true) {
        msg = "turn on Laser";
    } else {
        msg = "turn off Laser";
    }

    FT_STATUS ftStatus = FT_SetBitMode( ftHandle, currentBitSetVal.to_ulong(), 0x20 );  // Reset interface board
    LOG3(ftStatus, FT_OK, msg);
    if( ftStatus != FT_OK ) {
        qts << "Could not complete operation for Laser" << msg;
        LOG1(msg)
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
        return false;
    }

    return true;
}

bool InterfaceSupport::reportCurrentDeviceStatus() {

    UCHAR currentVal;
    FT_STATUS ftStatus = FT_GetBitMode(ftHandle, &currentVal);
    if( ftStatus != FT_OK ) {
        const QString msg( "Could not read current FTDI mode");
        LOG1(msg)
        return false;
    }

    std::bitset<8> currBitSetVal = currentVal;

    float hardwareVersion = getHardwareVersion();
    LOG1(hardwareVersion);
    float firmwareVersion = getFirmwareVersion();
    LOG1(firmwareVersion);
    float sledVersion = getSledFirmwareVersion();
    LOG1(sledVersion);

    LOG1(getVOASettings());

    LOG1(currBitSetVal.test(DEVICE_BIT_POSITION_INTERFACE_BOARD_RESET));

    LOG1(currBitSetVal.test(DEVICE_BIT_POSITION_SLED_5V));

    LOG1(currBitSetVal.test(DEVICE_BIT_POSITION_SLED_24V));

    LOG1(currBitSetVal.test(DEVICE_BIT_POSITION_LASER));

    float minimumACVoltage = (float) 122 * 0.9f;
    float detectedACVoltage = getSupplyVoltage();

    LOG2(minimumACVoltage, detectedACVoltage);

    if (minimumACVoltage > detectedACVoltage) {
        const QString msg("AC Power:    OFF");
        LOG1(msg)
    } else {
        const QString msg("AC Power:    ON");
        LOG1(msg)
    }

    int runState = getRunningState();
    QString sledRotation = "SLED OFF";
    switch (runState) {
    case 0:
        sledRotation = "STOPPED";
        break;
    case 1:
        sledRotation = "CLOCKWISE";
        break;
    case 3:
        sledRotation = "COUNTER-CLOCKWISE";
        break;
    default:
        break;
    }

    LOG1(sledRotation);

    return true;
}

QByteArray InterfaceSupport::readDataFromDevice(bool ignoreNakResponse) {
    QByteArray data;
    static QByteArray data0;
    if(ftHandle != nullptr) {
        DWORD bytesToRead = 256;
        DWORD bytesRead;
        char buffer[256]= {};

        Sleep(100);
        FT_STATUS ftStatus = FT_Read( ftHandle, buffer, bytesToRead, &bytesRead );
        if( ftStatus != FT_OK ) {
            const QString msg("Serial read failed");
            LOG1(msg)
        } else {
            buffer[bytesRead] = '\0';
            data = buffer;
            data = data.simplified();
        }

        if(!ignoreNakResponse && data.toUpper().contains( "NAK" )){
            LOG( INFO, "Device responded NAK");
        }
    }
    if(data0 != data){
        LOG1(data.toStdString().c_str())
        data0 = data;
    }
    return data;
}

bool InterfaceSupport::writeDataToDevice(QByteArray command) {

    static QByteArray command0;

    bool retVal = true;

    if(command0 != command){
        LOG1(command.toStdString().c_str())
        command0 = command;
    }
    if( ftHandle != NULL ) {
        DWORD bytesWritten;

        FT_STATUS ftStatus = FT_Purge( ftHandle, FT_PURGE_RX );   // flush input buffer
        if( ftStatus != FT_OK) {
            const QString msg( "Input flush failed");
            LOG1(msg)
        }

        Sleep(100);
        ftStatus = FT_Write( ftHandle, command.data(), command.size(), &bytesWritten);
        if( ftStatus != FT_OK ) {
            LOG( WARNING, QString( "Interface support: writeDataToDevice could not write command: %1 ").arg(command.data()));
            retVal = false;
        }
    } else {
        const QString msg( "Serial port not open for write");
        LOG1(msg)
        retVal = false;
    }

    return retVal;
}

void InterfaceSupport::updateSledConfig(const device &currentDevice)
{
    //void SledSupport::setSledParams( DeviceParams_T params )
    //{
    //        setClockingMode( params.isEnabled );//sc0 or sc1
    //        setClockingGain( params.gain ); //scg1:255
    //        setClockingOffset( params.offset );//sco1:999
    //        setSpeed( params.speed ); // bool setSledSpeed(int speed);
    //        setTorqueLimit( params.torque );// sto0:45
    //        setTorqueTime( params.time );//sti0:15
    //        setStallBlinking( params.blinkEnabled );//sb0:1
    //        setButtonMode( params.sledMulti ); //sbm0:2
    //}
    LOG1(currentDevice.getSplitDeviceName());

    setSledClockingEnabled(currentDevice);

    setSledClockingGain(currentDevice);

    setSledClockingOffset(currentDevice);

    setSledSpeed(currentDevice);

    setSledTorqueLimit(currentDevice);

    setSledTorqueTime(currentDevice);

    setSledStallBlinking(currentDevice);

    setSledButtonMode(currentDevice);
}

bool InterfaceSupport::setSledClockingEnabled(const device &currentDevice)
{
    bool success{false};
    QByteArray setClockingEnabledCmd{"sc"};
    int currentClockingEnabled{currentDevice.getClockingEnabled()};

    setClockingEnabledCmd.append(QByteArray(QString::number(currentClockingEnabled).toStdString().c_str())).append("\r");

    if(writeDataToDevice(setClockingEnabledCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, currentClockingEnabled);

    return success;
}

bool InterfaceSupport::setSledClockingGain(const device &currentDevice)
{
    bool success{false};

    QByteArray setClockingGainCmd{"scg"};
    QByteArray currentClockingGain{currentDevice.getClockingGain()};

    setClockingGainCmd.append(currentClockingGain).append("\r");
    if(writeDataToDevice(setClockingGainCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, setClockingGainCmd);

    return success;
}

bool InterfaceSupport::setSledClockingOffset(const device &currentDevice)
{
    bool success{false};
    QByteArray setClockingOffsetCmd{"sco"};
    QByteArray currentClockingOffset{currentDevice.getClockingOffset()};

    setClockingOffsetCmd.append(currentClockingOffset).append("\r");

    if(writeDataToDevice(setClockingOffsetCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, setClockingOffsetCmd);

    return success;
}

bool InterfaceSupport::setSledSpeed(const device &currentDevice)
{
    bool success{false};
    QByteArray setSpeedCmd{"ss"};
    int currentSpeed{currentDevice.getRevolutionsPerMin()};

    setSpeedCmd.append(QByteArray(QString::number(currentSpeed).toStdString().c_str())).append("\r");

    if(writeDataToDevice(setSpeedCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, currentSpeed);

    return success;
}

bool InterfaceSupport::setSledTorqueLimit(const device &currentDevice)
{
    bool success{false};
    QByteArray setTorqueLimitCmd{"sto"};
    QByteArray currentTorqueLimit{currentDevice.getTorqueLimit()};

    setTorqueLimitCmd.append(currentTorqueLimit).append("\r");
    if(writeDataToDevice(setTorqueLimitCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, setTorqueLimitCmd);

    return success;
}

bool InterfaceSupport::setSledTorqueTime(const device &currentDevice)
{
    bool success{false};
    QByteArray setTorqueTimeCmd{"sti"};
    QByteArray currentTorqueTime{currentDevice.getTorqueTime()};

    setTorqueTimeCmd.append(currentTorqueTime).append("\r");
    if(writeDataToDevice(setTorqueTimeCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, setTorqueTimeCmd);
    return success;
}

bool InterfaceSupport::setSledStallBlinking(const device &currentDevice)
{
    bool success{false};
    QByteArray setStallBlinkingCmd{"sb"};
    QByteArray currentStallBlinking{currentDevice.getStallBlinking()};

    setStallBlinkingCmd.append(currentStallBlinking).append("\r");
    if(writeDataToDevice(setStallBlinkingCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, setStallBlinkingCmd);

    return success;
}

bool InterfaceSupport::setSledButtonMode(const device &currentDevice)
{
    bool success{false};
    QByteArray setButtonModeCmd{"sbm"};
    QByteArray currentButtonMode{currentDevice.getButtonMode()};

    setButtonModeCmd.append(currentButtonMode).append("\r");
    if(writeDataToDevice(setButtonModeCmd)){
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            success = true;
        }
    }
    LOG2(success, setButtonModeCmd);


    return success;
}

int InterfaceSupport::getLastRunningState() const
{
    return m_lastRunningState;
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

bool InterfaceSupport::turnOnOffACPowerToOCT(bool isOn) {
    QByteArray command;

    LOG1(isOn)

    if (isOn) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_AC_POWER_ON_TO_OCT_ENGINE];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_AC_POWER_OFF_TO_OCT_ENGINE];
    }
    LOG1(command.toStdString().c_str())

    bool operationResult = false;

    if (writeDataToDevice(command)) {
        QByteArray response = readDataFromDevice();
        if(response.toUpper().contains("ACK")){
            operationResult = true;

            if (isOn) {
                LOG( INFO, "Successfully turned on AC power to OCT");
            } else {
                LOG( INFO, "Successfully turned off AC power to OCT");
            }
        } else {
            if (isOn) {
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
    LOG1(voaValue)
    return voaValue;
}

bool InterfaceSupport::setVOAMode(bool transparentMode) {
    QByteArray command;
    LOG1(transparentMode)

    if (transparentMode) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_VOA_TO_TRANSPARENT_MODE];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_VOA_TO_BLOCKING_MODE];
    }
    LOG1(command.toStdString().c_str())

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
    LOG1(speed)
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
    LOG1(command.toStdString().c_str())

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
    static int lastRunningState0;
    if (writeDataToDevice(interfaceBoardCommandList[OctInterfaceBoardCommandType::GET_SLED_RUNNING_STATE])) {
        bool ignoreNakResponse = true;
        QByteArray response = readDataFromDevice(ignoreNakResponse);
        if(response.toUpper().contains("ACK")){
            auto parts = QString(response).split(QLatin1Char(' '));
            for (auto part : parts) {
                if (part.contains("gr")) {
                    auto version = part.split(QLatin1Char('='));
                    m_lastRunningState = version[1].toInt();
//                    LOG( INFO, QString( "Interface support: getRunningState response: %1 ").arg(runState));
                    break;
                }
            }
        }
    }
    if(lastRunningState0 != m_lastRunningState){
        LOG1(m_lastRunningState)
        lastRunningState0 = m_lastRunningState;
    }
    return m_lastRunningState;
}

bool InterfaceSupport::setSledRunState(bool runState) {
    QByteArray command;

    if (runState) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::START_SLED];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::STOP_SLED];
    }
    LOG1(command.toStdString().c_str())

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

    LOG1(direction)

    if (direction) {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_CLOCKWISE_DIRECTION];
    } else {
        command = interfaceBoardCommandList[OctInterfaceBoardCommandType::SET_COUNTER_CLOCKWISE_DIRECTION];
    }
    LOG1(command.toStdString().c_str())

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

//void SledSupport::setSledParams( DeviceParams_T params )
//{
//        setClockingMode( params.isEnabled );//sc0 or sc1
//        setClockingGain( params.gain ); //scg1:255
//        setClockingOffset( params.offset );//sco1:999
//        setSpeed( params.speed ); // bool setSledSpeed(int speed);
//        setTorqueLimit( params.torque );// sto0:45
//        setTorqueTime( params.time );//sti0:15
//        setStallBlinking( params.blinkEnabled );//sb0:1
//        setButtonMode( params.sledMulti ); //sbm0:2
//}
