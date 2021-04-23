#ifndef INTERFACESUPPORT_H
#define INTERFACESUPPORT_H

#include "ftd2xx.h"
#include<bitset>
#include <QByteArray>
#include <map>

/*
* Enum declarations for various commands to be executed and status information
* to be queried from L300 interface board.
*/
enum class OctInterfaceBoardCommandType {
    GET_HARDWARE_VERSION = 1,
    GET_FIRMWARE_VERSION = 2,
    GET_SUPPLY_VOLTAGE = 3,
    SET_AC_POWER_OFF_TO_OCT_ENGINE = 4,
    SET_AC_POWER_ON_TO_OCT_ENGINE = 5,
    GET_VOA_SETTINGS = 6,
    SET_VOA_TO_TRANSPARENT_MODE = 7,
    SET_VOA_TO_BLOCKING_MODE = 8,
    GET_SLED_FIRMWARE_VERSION = 9,
    SET_SLED_SPEED = 10,
    SET_BIDIRECTIONAL_MODE_OFF = 11,
    SET_BIDIRECTIONAL_MODE_ON = 12,
    GET_SLED_RUNNING_STATE = 13,
    START_SLED = 14,
    STOP_SLED = 15,
    SET_CLOCKWISE_DIRECTION = 16,
    SET_COUNTER_CLOCKWISE_DIRECTION = 17,
};

using InterfaceBoardCommandList = std::map<OctInterfaceBoardCommandType, QByteArray>;

/*
* InterfaceSupport class provides various interfaces to control parameters and functions to L300 interface board.
*/
class InterfaceSupport
{

public:
    /*
     * Creates or gets instance to InterfaceSupport
     *
     * @param resetInterfaceBoard
     *      Flag if set to true will force resetting of interface board.
     *
     * @return singleton instance to InterfaceSupport class
    */
    static InterfaceSupport* getInstance(bool resetInterfaceBoard = false);

    /*
     * Releases InterfaceSupport singleton instance
     *
     * @return True if intialization is successful
    */
    static void releaseInstance();

    /*
     * Reset interface board and reboot microprocessor.
     * FTDI I/O pin to reset interface board to both low and high
     *
     * @return True if reset is successful
     */
    bool resetInterfaceBoard();

    /*
     * Reset interface board. Effect is turn off Fan
     *
     * @return True if reset is successful
     */
    bool turnOffInterfaceBoard();

    /*
     * Turn on Sled 5V.
     *
     * @param isOn
     *      enabling state of Sled 5V to be set.
     *
     * @return True if sled is turned on or off successfullly
     */
    bool turnOnOffSled5V(bool isOn);

    /*
     * Turn on Sled 24V.
     *
     * @param isOn
     *      enabling state of Sled 24V to be set.
     *
     * @return True if sled is turned on or off successfullly
     */
    bool turnOnOffSled24V(bool isOn);

    /*
     * Turn on Sled.
     *
     * @param isOn
     *      enabling state of Sled for both 5V and 24V to be set.
     *
     * @return True if sled is turned on or off successfullly
     */
    bool turnOnOffSled(bool isOn);

    /*
     * Turn on Laser.
     *
     * @param state
     *      enabling state of laser to be set.
     *
     * @return True if laser is turned on successfullly
     */
    bool turnOnOffLaser(bool isOn);

    /*
     * Reset interface board low. The effect is; fans get turned OFF
     *
     * @return True if laser is turned on successfullly
     */
    bool performResetLow();

    /*
     * Reset interface board high. The effect is; fans get turned ON
     *
     * @return True if laser is turned on successfullly
     */
    bool performResetHigh();

    /*
     * Queries current device settings from the device and report it
     *
     * @return True status can be queried successfullly
     */
    bool reportCurrentDeviceStatus();

    /*
     * Queries hardware version from interface board
     *
     * @returns hardware version in float value
     */
    float getHardwareVersion();

    /*
     * Queries firmware version from interface board
     *
     * @returns firmware version in float value
     */
    float getFirmwareVersion();

    /*
     * Queries supply voltage to determine if AC power is available to the interface board
     *
     * @returns supply voltage in float value
     */
    float getSupplyVoltage();

    /*
     * Enables both Sled and Laser
     *
     * @return True if sled and laser are enabled successfullly
     */
    bool turnOnSledAndLaser();

    /*
     * Queries Sled firmware version from interface board
     *
     * @returns Sled firware version in float
     */
    float getSledFirmwareVersion();

    /*
     * Turn on AC power to the OCT
     *
     * @param state
     *      enabling state of AC power to the OCT.
     *
     * @return True if AC power to the OCT can be turned on successfullly
     */
    bool turnOnACPowerToOCT(bool state);

    /*
     * Queries VOA settings from the interface board
     *
     * @returns VOA settings
     */
    int getVOASettings();

    /*
     * Set VOA mode to either transparent or blocking mode
     *
     * @param transparentMode
     *      True if mode needs to be set to transparent; False if mode to be set as blocking
     *
     * @returns True if VOA mode can be set successfully
     */
    bool setVOAMode(bool transparentMode);

    /*
     * Set Sled to operate on a supplied speed
     *
     * @param speed
     *      Speed of Sled to be set
     *
     * @returns True if speed can be set successfully
     */
    bool setSledSpeed(int speed);

    /*
     * Set Bi-directional mode for the device
     *
     * @param bidirectionalState
     *      Flag is set to True if bi-directional state needs to be enabled on the device
     *
     * @returns True if bi-directional state can be set successfully
     */
    bool enableDisableBidirectional(bool bidirectionalState);

    /*
     * Set running state of Sled
     *
     * @param runState
     *      Flag is set to True if Sled needs to be run after it is turned on.
     *
     * @returns True if running state can be set successfully
     */
    bool setSledRunState(bool runState);

    /*
     * Set running direction for Sled
     *
     * @param direction
     *      Flag is set to True if Sled direction is to be set clockwise.
     *      Flag is set to False if Sled direction is to be set counterclockwise.
     *
     * @returns True if supplied running state can be set successfully
     */
    bool setSledDirection(bool direction);

    /*
     * Queries running state of Sled
     *
     * @returns run state
     */
    int getRunningState();

    int getLastRunningState() const;

private:

    /*
    * @name Instance should be created only through singlton interface.
    *
    * No copying and moving allowed.
    */
    InterfaceSupport() = default;
    ~InterfaceSupport();
    InterfaceSupport(const InterfaceSupport&) = delete;
    InterfaceSupport& operator=(const InterfaceSupport&) = delete;

    InterfaceSupport(InterfaceSupport&&) = delete;
    InterfaceSupport& operator=(InterfaceSupport&&) = delete;

    /*
     * Device needs to be preped after it is opned through initalizeFTDIDevice call.
     * baud rate will be set for the serial communication to receive responses
     */
    void prepDevice();

    /*
     * Initializes a FTDI device and dumps device info
     *
     * @return True if intialization is successful
    */
    bool initalizeFTDIDevice();

    /*
     * Build a list of commands to be sent to the interface board to query various paramters
     * and to execute commands
     */
    void populateInterfaceBoardCommandList();

    /*
     * Reads command response from the interface board
     *
     * @param ignoreNakResponse
     *      If the flag is set to true, do not log any message if we get NAK response from the
     *      interface board. Usually, we may need to ignore NAK response when we query the
     *      running state of sled.
     *
     * @returns supply voltage in float value
     */
    QByteArray readDataFromDevice(bool ignoreNakResponse = false);

    /*
     * Queries supply voltage to determine if AC power is available to the interface board
     *
     * @param command
     *      command string in byte array to be written to the serial port to get the response.
     *
     * @returns True if write is successful
     */
    bool writeDataToDevice(QByteArray command);

    FT_DEVICE_LIST_INFO_NODE *ftdiDeviceInfo = nullptr; // FTDI device information
    FT_HANDLE ftHandle = nullptr; // FTDI handle to be used to read/write bits
    std::bitset<8> currentBitSetVal; // Bitset used to control device functionality

    static InterfaceSupport* m_instance; // Pointer to singleton instance to InterfaceSupport class

    InterfaceBoardCommandList interfaceBoardCommandList; // List of commands to be used to query interface board

    int m_lastRunningState{0};
};

#endif // INTERFACESUPPORT_H
