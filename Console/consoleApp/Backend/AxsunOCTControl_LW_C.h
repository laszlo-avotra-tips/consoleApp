// AxsunOCTControl_LW_C.h
// Copyright 2020 Axsun Technologies

/** \file AxsunOCTControl_LW_C.h
\brief This header file defines the usage of the %AxsunOCTControl_LW "lightweight" library in a C or C++ application.
 
Refer to the Main Page for \link cusagesec C API Usage Guidelines \endlink.
 */

#ifndef AXSUNOCTCONTROLLW_C_H
#define AXSUNOCTCONTROLLW_C_H

#include "AxsunCommonEnums.h"

#ifdef LabVIEW_CALLBACK
#include "extcode.h"		// only required for LabVIEW callback integration
#endif

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif // __cplusplus

/*! \brief DAQ clock and trigger sources. */
typedef enum {
	EXTERNAL = 0,	/*!< External source (i.e. laser engine trigger and k-clock). */
	INTERNAL = 1,	/*!< Internally-generated source (e.g. for simulation purposes). */
	LVDS = 2,		/*!< External LVDS (for Image_sync trigger only). */
	LVCMOS = 3,		/*!< External LVCMOS (for Image_sync trigger only). */
} AxEdgeSource;

typedef enum {
	NONE = 0,
	USB = 1,					/*!< USB 2.0 interface. */
	RS232_PASSTHROUGH = 2,		/*!< via DAQ's Ethernet interface. */
	RS232 = 3,					/*!< RS-232 serial interface. */
	ETHERNET = 4				/*!< TCP/IP Ethernet interface. */
} AxConnectionType;

typedef enum {
	UNDEFINED = 0,
	LASER = 40,			/*!< Axsun Laser Engine Device. */
	CLDAQ = 41,			/*!< Axsun Camera Link DAQ Device. */
	EDAQ = 42			/*!< Axsun Ethernet/PCIe DAQ Device. */
} AxDevType;

/*! \brief Laser TEC states. */
typedef enum {
	TEC_UNINITIALIZED = 0,			/*!< TEC has not been initialized. */
	WARMING_UP = 1,					/*!< TEC is stabilizing. */
	WAITING_IN_RANGE = 2,			/*!< TEC is stabilizing. */
	READY = 3,						/*!< TEC is ready. */
	NOT_INSTALLED = 5,				/*!< TEC is not installed. */
	ERROR_NEVER_GOT_TO_READY = 16,	/*!< TEC error: never got to ready state. */
	ERROR_WENT_OUT_OF_RANGE = 17	/*!< TEC error: temperature went out of range. */
} AxTECState;

/*! \brief Type defintion for a user-provided function to be called upon connection or disconnection of a device. */
typedef void(__cdecl *AxConnectCallbackFunctionC_t)(void*);

/**
 \brief Gets a string which explains an error code in a more verbose fashion.
 \param errornum An error code number returned from other "ax" functions in this library.
 \param error_string A char array pre-allocated with at least 512 bytes into which the error explanation will be copied as a nul-terminated string.
 \details axGetErrorExplained() can be called at any time. It is unsafe to pass a error_string output buffer allocated with fewer than 512 bytes.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
void axGetErrorExplained(AxErr errornum, char* error_string);

/**
 \brief Opens the AxsunOCTControl context for subsequent device communication.
 \param open_all_interfaces If TRUE the USB and Ethernet interfaces will be started automatically. If FALSE no interfaces will be started.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details See \link cusagesec C API Usage Guidelines \endlink on the Main Page for more information.
 \details If interfaces are not opened at the time of this function call, they can be opened later using axNetworkInterfaceOpen(), axUSBInterfaceOpen(), or axSerialInterfaceOpen(). This allows the registration of the connect/disconnect callback function using axRegisterConnectCallback() _prior to_ opening interfaces and gives the user individual control over which interface(s) to use.
 \details Close an AxsunOCTControl context by calling axCloseAxsunOCTControl().
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axOpenAxsunOCTControl(uint32_t open_all_interfaces);

/**
 \brief Closes an AxsunOCTControl context previously opened with axOpenAxsunOCTControl().
 \return NO_AxERROR on success or other AxErr error code on failure. */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axCloseAxsunOCTControl();

/**
 \brief Opens, resets, or closes the Ethernet network interface.
 \param interface_status =1 opens the interface or resets an existing open interface, =0 closes the interface.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details When the interface is open, devices present on this interface will be detected, enumerated, and trigger the execution of a callback function registered using axRegisterConnectCallback(). Use this function if the interface was not opened automatically during the initial axOpenAxsunOCTControl() call.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axNetworkInterfaceOpen(uint32_t interface_status);

/**
 \brief Opens, resets, or closes the USB interface.
 \param interface_status =1 opens the interface or resets an existing open interface, =0 closes the interface.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details When the interface is open, devices present on this interface will be detected, enumerated, and trigger the execution of a callback function registered using axRegisterConnectCallback(). Use this function if the interface was not opened automatically during the initial axOpenAxsunOCTControl() call.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axUSBInterfaceOpen(uint32_t interface_status);

/**
 \brief Opens, resets, or closes a RS-232 serial interface on a given port.
 \param interface_status =1 opens an interface or resets an existing open interface, =0 closes an interface if a `port` is provided, or closes all interfaces if `port` is "" (empty string).
 \param port Null-terminated char array describing the name of the port (e.g. typically "COM_" on Windows or "/dev/tty.___" on Linux/macOS)
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details When the interface is open, a device present on this interface will be detected, enumerated, and trigger the execution of a callback function registered using axRegisterConnectCallback().
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSerialInterfaceOpen(uint32_t interface_status, char* port);

/**
\brief Control the DAQ operational mode (Live Imaging, Burst Recording, or Imaging Off) when using the Ethernet interface.
\param number_of_images =0 for Imaging Off (idle), =-1 for Live Imaging (no record), or any positive value between 1 and 32767 to request the desired number of images in a Burst Record operation.
\param which_DAQ The numeric index of the desired DAQ.
\return NO_AxERROR on success or other AxErr error code on failure.
\details This function does NOT control the laser and therefore the laser emission must be enabled/disabled separately using axSetLaserEmission(). To control the DAQ operational mode when using the PCIe interface, call axImagingCntrlPCIe() in the AxsunOCTCapture library.
*/
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axImagingCntrlEthernet(int16_t number_of_images, uint32_t which_DAQ);

/**
 \brief Select the Image_sync source.
 \param source The desired Image_sync source. Must be either INTERNAL, LVCMOS, or LVDS.
 \param frequency The Image_sync frequency (Hz); this parameter is optional and is ignored when source is not INTERNAL.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Use INTERNAL Image_sync source only for simulation and troubleshooting.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetImageSyncSource(AxEdgeSource source, float frequency, uint32_t which_DAQ);

/**
 \brief Select the ADC sample clock source.
 \param source The desired sample clock source. Must be either EXTERNAL (Laser k-clock) or INTERNAL (500 MS/s).
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Use INTERNAL sample clock source only for simulation and troubleshooting.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetSampleClockSource(AxEdgeSource source, uint32_t which_DAQ);

/**
 \brief Select the sweep trigger source.
 \param source The desired sweep trigger source. Must be either EXTERNAL (from Laser) or INTERNAL (100 kHz).
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Use INTERNAL sweep trigger source only for simulation and troubleshooting.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetSweepTriggerSource(AxEdgeSource source, uint32_t which_DAQ);

/**
 \brief Select the DAQ's pipeline mode (i.e. bypass mode) and polarization channel configuration.
 \param pipeline_mode The desired pipeline mode.
 \param polarization_mode The desired polarization channel (horizontal, vertical, diverse, or interleaved).
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Insure that A-line subsampling has been configured with SetSubsamplingFactor() \b prior \b to setting a pipeline mode which could saturate the data interface bandwidth if subsampling is insufficient.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetPipelineMode(AxPipelineMode pipeline_mode, AxChannelMode polarization_mode, uint32_t which_DAQ);

/**
 \brief Set the GAIN term during 16- to 8-bit dynamic range compression.
 \param gain Gain must be between 0 and 15.999756.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetEightBitGain(float gain, uint32_t which_DAQ);

/**
 \brief Set the OFFSET term during 16- to 8-bit dynamic range compression.
 \param offset Offset must be between -128.0 and 127.996094.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetEightBitOffset(float offset, uint32_t which_DAQ);

/**
 \brief Set the A-line subsampling factor.
 \param subsampling_factor Subsampling factor must be between 1 and 166.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetSubsamplingFactor(uint8_t subsampling_factor, uint32_t which_DAQ);

/**
 \brief Gets (i.e. reads) the current value in a FPGA register.
 \param regnum The FPGA register number to read.
 \param regval The value read back from the FPGA register.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetFPGARegister(const uint32_t regnum, uint16_t* regval, uint32_t which_DAQ);

/**
 \brief Gets (i.e. reads) current values in a range of FPGA registers.
 \param start_regnum The first FPGA register to read in the range.
 \param end_regnum The last FPGA register to read in the range; must be greater than or equal to `start_regnum`.
 \param regvals Pointer to a pre-allocated array to hold the 16-bit register values read back from the requested range of FPGA registers.
 \param bytes_allocated The number of bytes pre-allocated by the user in the array pointed to by `regvals`.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details The size of the pre-allocated memory pointed to by `regvals` must be 2 bytes per register in the included range:  `bytes_allocated` should equal `2 * (end_regnum - start_regnum + 1)`.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetFPGARegisterRange(const uint32_t start_regnum, const uint32_t end_regnum, uint16_t* regvals, const uint32_t bytes_allocated, uint32_t which_DAQ);


/**
 \brief Gets (i.e. reads) the programmed power-on default FPGA register configuration script as register number-value pairs.
 \param elements_allocated Indicates the size (<b>in number of U16 elements, NOT bytes</b>) of the pre-allocated `regnums` and `regvals` arrays.
 \param regnums Pointer to a pre-allocated array to hold the 16-bit register numbers read back from the device.
 \param regvals Pointer to a pre-allocated array to hold the 16-bit register values read back from the device.
 \param elements_returned The number of register number-value pairs read back from the device.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details `regnums` and `regvals` are arrays which must be pre-allocated by the user. `elements_returned` will never be higher than `elements_allocated`, however if `elements_returned == elements_allocated`, there may be more register number-value pairs stored on the device which were _not_ returned (indicating `elements_allocated` was too small to hold the full FPGA register configuration script).
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetFPGARegisterDefaults(uint32_t elements_allocated, uint16_t* regnums, uint16_t* regvals, uint32_t* elements_returned, uint32_t which_DAQ);


/**
 \brief Sets (i.e. writes) a power-on default FPGA register configuration script, overwriting the existing script.
 \param regnums Pointer to an array containing 16-bit register numbers.
 \param regvals Pointer to an array containing 16-bit register values.
 \param count The number of register number-value pairs to be written to the device.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details `regnums` points to an array containing `count` register numbers and `regvals` points to an equal-sized array of corresponding register default values.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetFPGARegisterDefaults(const uint16_t* regnums, const uint16_t* regvals, const uint32_t count, uint32_t which_DAQ);

/**
 \brief Sets (i.e. writes) a FPGA register with a single value.
 \param regnum The FPGA register number in which to write.
 \param regval The value to be written.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetFPGARegister(const uint32_t regnum, const uint16_t regval, uint32_t which_DAQ);

/**
 \brief Sets or Clears a single bit in a FPGA register.
 \param regnum The FPGA register number in which to set or clear a bit.
 \param which_bit The bit number in the interval [0..15] to set or clear.
 \param value =1 to Set the bit, =0 to Clear the bit.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetFPGARegisterSingleBit(const uint32_t regnum, const uint32_t which_bit, const uint32_t value, uint32_t which_DAQ);

/**
\brief Sets (i.e. writes) the low or high byte of an FPGA register, leaving the other byte unchanged.
\param regnum The FPGA register number in which to write.
\param which_byte Set to = 0 for low byte, = 1 for high byte.
\param value The value to be written. 
\param which_DAQ The numeric index of the desired DAQ.
\return NO_AxERROR on success or other AxErr error code on failure.
*/
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetFPGARegisterSingleByte(const uint32_t regnum, const uint32_t which_byte, const uint32_t value, uint32_t which_DAQ);

/**
\brief Sets (i.e. writes) one nibble (4 bits) of an FPGA register, leaving the other 12 bits unchanged.
\param regnum The FPGA register number in which to write.
\param which_nibble Set to = 0 for lowest nibble, ... , = 3 for highest nibble.
\param value The value to be written (must be in range of 0 to 15).
\param which_DAQ The numeric index of the desired DAQ.
\return NO_AxERROR on success or other AxErr error code on failure.
*/
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetFPGARegisterSingleNibble(const uint32_t regnum, const uint32_t which_nibble, const uint32_t value, uint32_t which_DAQ);

/**
 \brief Sets (i.e. writes) a FPGA register with an array of multiple values.
 \param regnum The FPGA register number in which to write.
 \param data_array Pointer to an array containing 16-bit values to be written.
 \param number_of_words The number of 16-bit register values pointed to by `data_array`.
 \param which_DAQ The numeric index of the desired DAQ.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetFPGADataArray(const uint32_t regnum, const uint16_t* data_array, const uint32_t number_of_words, uint32_t which_DAQ);

/**
 \brief Reads the current value in a DAQ PHY register.
 \param phynum The PHY to read.
 \param regnum The PHY register number to read.
 \param regval The value read back from the PHY register.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details This function is for Axsun internal use only.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axReadDAQPHYRegister(uint32_t phynum, uint32_t regnum, uint16_t* regval, uint32_t which_DAQ);

/**
 \brief Disable or re-enable the Connection Heartbeat for a device connected by Ethernet network or RS-232.
 \param heartbeat_state =1 to enable the Heartbeat, =0 to disable the Heartbeat.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details The Connection Heartbeat is enabled by default when a device is connected. This function can be used to disable or subsequently re-enable it. The Connection Heartbeat sends a periodic, non-functional message to a connected device for determining its current connectivity status. If a Heartbeat response is not received, a device disconnection has occurred. If the connection Heartbeat is disabled, a disconnected device will not be recognized as such until the response to a subsequent functional message is not received.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axConnectionHeartbeat(uint32_t heartbeat_state, uint32_t which_device);

/**
 \brief Send a debug command.
 \param command_number
 \param value_out
 \param command_in
 \param value_in
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details This function is for Axsun internal use only.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axDebugCommand(uint32_t command_number, uint32_t value_out, uint32_t* command_in, uint32_t* value_in, uint32_t which_device);

/**
 \brief Gets the device firmware version.
 \param major Version number major element.
 \param minor Version number minor element.
 \param patch Version number patch element.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axFirmwareVersion(uint32_t* major, uint32_t* minor, uint32_t* patch, uint32_t which_device);

/**
 \brief Gets the device FPGA version.
 \param major Version number major element.
 \param minor Version number minor element.
 \param patch Version number patch element.
 \param build Version number build element.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Devices which do not have an FPGA will return all version elements = 0.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axFPGAVersion(uint32_t* major, uint32_t* minor, uint32_t* patch, uint32_t* build, uint32_t which_device);

/**
 \brief Gets the library version.
 \param major Version number major element.
 \param minor Version number minor element.
 \param patch Version number patch element.
 \param build Version number build element.
 \return NO_AxERROR on success.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axLibraryVersion(uint32_t* major, uint32_t* minor, uint32_t* patch, uint32_t* build);

/**
 \brief Gets the device type (e.g. Laser, EDAQ).
 \param device_type The type of device.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axDeviceType(AxDevType * device_type, uint32_t which_device);

/**
 \brief Gets the device serial number string.
 \param serial_number A char array pre-allocated with at least 40 bytes into which the device serial number will be copied as a null-terminated string.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSerialNumber(char * serial_number, uint32_t which_device);

/**
 \brief Gets the device connection interface (e.g. USB, Ethernet, RS-232).
 \param connection_type The interface on which the device is connected.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axConnectionType(AxConnectionType * connection_type, uint32_t which_device);

/**
 \brief Counts the number of individual device settings elements for each of the three data types (float, int, and bool).
 \param float_count The number of floating point settings.
 \param int_count The number of integer settings.
 \param bool_count The number of boolean settings.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axCountDeviceSettings(uint32_t* float_count, uint32_t* int_count, uint32_t* bool_count, uint32_t which_device);

/**
 \brief Gets a floating point setting's value and name string.
 \param which_setting The index of a floating point setting.
 \param setting_value The floating point setting value.
 \param setting_string A char array pre-allocated with at least 40 bytes into which the setting name will be copied as a null-terminated string.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Use axCountDeviceSettings() to determine the number of floating point settings available (`float_count`) and then call this function in a loop while indexing `which_setting` from `0` up to `float_count - 1`.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetFloatSetting(uint32_t which_setting, float* setting_value, char * setting_string, uint32_t which_device);

/**
 \brief Gets a integer setting's value and name string.
 \param which_setting The index of an integer setting.
 \param setting_value The integer setting value.
 \param setting_string A char array pre-allocated with at least 40 bytes into which the setting name will be copied as a null-terminated string.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Use axCountDeviceSettings() to determine the number of integer settings available (`int_count`) and then call this function in a loop while indexing `which_setting` from `0` up to `int_count - 1`.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetIntSetting(uint32_t which_setting, int32_t* setting_value, char * setting_string, uint32_t which_device);

/**
 \brief Gets a boolean setting's value and name string.
 \param which_setting The index of a boolean setting.
 \param setting_value The boolean setting value (non-zero if TRUE, zero if FALSE).
 \param setting_string A char array pre-allocated with at least 40 bytes into which the setting name will be copied as a null-terminated string.
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Use axCountDeviceSettings() to determine the number of boolean settings available (`bool_count`) and then call this function in a loop while indexing `which_setting` from `0` up to `bool_count - 1`.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetBoolSetting(uint32_t which_setting, uint8_t* setting_value, char * setting_string, uint32_t which_device);

/**
 \brief Write device settings to non-volatile memory (i.e. flash).
 \param float_count The number of floating point settings in the `float_values` array.
 \param int_count The number of integer settings in the `int_values` array.
 \param bool_count The number of boolean settings in the `bool_values` array.
 \param float_values An array of floating point setting values.
 \param int_values An array of integer setting values.
 \param bool_values An array of boolean setting values (1 = TRUE, 0 = FALSE).
 \param which_device The numeric index of the desired device.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details After reading all device setting values and names (using axCountDeviceSettings(), axGetFloatSetting(), axGetIntSetting(), and axGetBoolSetting()), original (i.e. unchanged) and new (i.e. changed or updated) settings are written back to the device's non-volatile memory with this function.  Do \b NOT change the ordering or indexing of the device settings. The original values of unchanged settings \b must be written back to the device with the changed ones.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSettingsToDevice(uint32_t float_count, uint32_t int_count, uint32_t bool_count, float* float_values, int32_t* int_values, uint8_t* bool_values, uint32_t which_device);

/**
 \brief Counts the number of devices successfully connected and enumerated by AxsunOCTControl.
 \return The number of connected devices.
 \details Count includes Laser engine and DAQ boards. A device connected on two interfaces concurrently will be counted twice and thus is not a recommended configuration.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
uint32_t axCountConnectedDevices();

/**
 \brief Enables or disables swept laser emission.
 \param emission_state =1 enables laser emission, =0 disables laser emission.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetLaserEmission(uint32_t emission_state, uint32_t which_laser);

/**
 \brief Gets swept laser emission status.
 \param emitting Will be non-zero if laser is emitting or =0 if laser is not emitting.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetLaserEmission(uint32_t* emitting, uint32_t which_laser);

 /**
 \brief Enables or disables pointer/aiming laser emission.
 \param emission_state =1 enables pointer/aiming laser emission, =0 disables pointer/aiming laser emission.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
 /*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetPointerEmission(uint32_t emission_state, uint32_t which_laser);

 /**
 \brief Gets pointer/aiminglaser emission status.
 \param emitting Will be non-zero if pointer/aiming laser is emitting or =0 if pointer/aiming laser is not emitting.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetPointerEmission(uint32_t* emitting, uint32_t which_laser);

/**
 \brief Sets the electronic K-clock delay.
 \param delay_code A 6-bit code representing the desired delay value.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details <TODO> SOME DETAILS HERE
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetClockDelay(uint32_t delay_code, uint32_t which_laser);

/*! \brief Gets the currently configured electronic K-clock delay.
 \param delay_code Will contain a 6-bit code representing the configured delay value.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details See details for axSetClockDelay().
*/
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetClockDelay(uint32_t* delay_code, uint32_t which_laser);

/**
 \brief Starts the VDL home operation.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details This function returns after _requesting_ the homing operation from the VDL controller; it does not wait for the mechanical homing operation to complete before returning.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axHomeVDL(uint32_t which_laser);

/**
 \brief Stops any VDL operation (e.g. move or home) currently in progress.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axStopVDL(uint32_t which_laser);

/**
 \brief Move the VDL relative to its current position.
 \param rel_position The distance to move from the current position in mm.
 \param speed The speed at which to move in mm/sec (maximum =7.5).
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axMoveRelVDL(float rel_position, float speed, uint32_t which_laser);

/**
 \brief Move the VDL to an absolute position.
 \param abs_position The new desired position in mm.
 \param speed The speed at which to move in mm/sec (maximum =7.5).
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axMoveAbsVDL(float abs_position, float speed, uint32_t which_laser);

/**
 \brief Gets the current status of the VDL.
 \param current_pos Current VDL position (mm).
 \param target_pos Target VDL position (mm).  Will be different than the current position during a move or home operation.
 \param speed Current speed of VDL (mm/sec).
 \param error_from_last_home Accumulated error since last home operation (number of microsteps).
 \param last_move_time Time for last move operation (sec).
 \param state Integer codes to represent VDL state.
										*	- 1 = idle
										*	- 2 = moving
										*	- 3 = seeking home
										*	- 4 = backing off home
										*	- 5 = at home
										*	- 10 = error state
 \param home_switch Non-zero if home (minimum position) limit switch is activated, =0 otherwise.
 \param limit_switch Non-zero if maximum position limit switch is activated, =0 otherwise.
 \param VDL_error Debugging information for VDL errors.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Refer to documentation on AxsunOCTControl_LW::VDLStatus for additional explanation of function arguments.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetVDLStatus(float* current_pos, float* target_pos, float* speed,
	int32_t* error_from_last_home, uint32_t* last_move_time,
	uint8_t* state, uint8_t* home_switch, uint8_t* limit_switch, uint8_t* VDL_error, uint32_t which_laser);

/**
 \brief Gets the current state of a Laser Thermo-Electric Cooler (TEC).
 \param TEC_state Current state of the queried laser TEC as described by the AxsunOCTControl_LW::TECState enum.
 \param which_TEC The index of which TEC to query (1 or 2).
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Some Laser Engines support more than one TEC. To query the state of the primary "TEC 1", use `which_TEC` = 1, or for a secondary TEC use `which_TEC` = 2.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetTECState(AxTECState* TEC_state, uint32_t which_TEC, uint32_t which_laser);

/**
 \brief Sets the DAC table.
 \param points
 \param speed
 \param timer1time
 \param timer2time
 \param timer1duration
 \param timer2duration
 \param currents
 \param voltages
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details This function is for Axsun internal use only.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetDACTable(uint32_t points, uint32_t speed, uint32_t timer1time, uint32_t timer2time,
	uint32_t timer1duration, uint32_t timer2duration, uint16_t* volts_currents, uint32_t which_laser);

/**
 \brief Gets the currently configured DAC table.
 \param points
 \param speed
 \param timer1time
 \param timer2time
 \param timer1duration
 \param timer2duration
 \param currents
 \param voltages
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details This function is for Axsun internal use only.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetDACTable(uint32_t* points, uint32_t* speed, uint32_t* timer1time, uint32_t* timer2time,
	uint32_t* timer1duration, uint32_t* timer2duration, uint16_t* volts_currents, uint32_t which_laser);

/**
\brief Selects a pre-programmed laser drive configuration.
\param which_config The desired configuration number (0, 1, 2, or 3).
\param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
\details Laser Engines must be programmed during manufacture to support multiple drive configurations.  Contact Axsun technical support for more information.
*/
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axSetDriveConfiguration(uint32_t which_config, uint32_t which_laser);

/**
 \brief Gets the currently selected drive configuration.
 \param current_configuration The currently selected drive configuration.
 \param which_laser The numeric index of the desired Laser.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details Laser Engines must be programmed during manufacture to support multiple drive configurations.  Contact Axsun technical support for more information.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axGetDriveConfiguration(uint32_t * current_configuration, uint32_t which_laser);

/**
 \brief Registers a callback function to be executed following a device connection or disconnection event.
 \param callback_function A user-supplied function to be called.
 \param userData An arbitrary structure to be made available inside the callback function (or NULL if unused).
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details The supplied callback function will be executed by a background thread when a new device is detected or when an existing device is disconnected. Within the callback function, access is provided to arbitrary user data passed as a void* parameter to this function.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axRegisterConnectCallback(AxConnectCallbackFunctionC_t callback_function, void * userData);

#ifdef LabVIEW_CALLBACK
/**
 \brief Registers a LabVIEW user event to be executed following a device connection or disconnection event.
 \param refnumptr A refnum pointer of a LabVIEW user event.
 \return NO_AxERROR on success or other AxErr error code on failure.
 \details LabVIEW code can be called directly from this library when a new device is detected or when an existing device is disconnected, similar to a C-based callback function registered using axRegisterConnectCallback(). See the %AxsunOCTControl_LW LabVIEW example project and _extcode.h_ header file (provided by National Instruments) for more details.
 */
/*! \cond */ AXSUN_EXPORTS /*! \endcond */
AxErr axRegisterLabVIEWCallback(LVUserEventRef* refnumptr);
#endif

#ifdef __cplusplus
}		// end extern "C"
#endif // __cplusplus

#endif		/* End header include guard */

