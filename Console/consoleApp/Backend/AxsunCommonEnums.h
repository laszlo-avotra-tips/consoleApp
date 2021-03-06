/** \file AxsunCommonEnums.h
* \brief 
This header file contains enums and other definitions for integrating _AxsunOCTCapture.dll_ and _AxsunOCTControl_LW.dll_ into a client application.
*/

#ifndef AXSUNCOMMONENUMS_H
#define AXSUNCOMMONENUMS_H

#ifdef _WIN32
/*! \cond */ #define AXSUN_EXPORTS __declspec(dllexport) /*! \endcond */
#else	// linux or macOS
#define AXSUN_EXPORTS __attribute__((visibility("default")))
#include <stddef.h>				// for size_t on linux
#define __cdecl					// re-define "__cdecl" to be "" for non-Windows targets
#endif // _WIN32

#ifdef __cplusplus
#include <cstdint>			// for integer typedefs
#else
#include <stdint.h>			// for integer typedefs
#endif // __cplusplus

/** \brief Structure for packet time stamp with platform-independent size (for internal use.) */
typedef struct axtimeval_t {
	int32_t tv_sec;
	int32_t tv_usec;
} axtimeval_t;

/** \brief Error codes returned from AxsunOCTCapture or AxsunOCTControl_LW functions. Use axGetErrorString() in AxsunOCTCapture or axGetErrorExplained() in AxsunOCTControl_LW for descriptions. */
typedef enum
#ifdef __cplusplus
class
#endif // __cplusplus
AxErr {
	NO_AxERROR = 0,						
	UNKNOWN_ERROR = -10004,				
	INVALID_ARGUMENTS = -10007,			
	CAPABILITY_NOT_SUPPORTED = -10009,
	UNDEFINED_BOARD_TYPE = -10013,

	CAPTURE_SESSION_NOT_SETUP = -5000,
	CAPTURE_SESSION_ALREADY_DESTROYED = -5001,
	CAPTURE_INTERFACE_SETUP_FAILURE = -5002,
	CAPTURE_SESSION_HANDLE_INVALID = -5003,
	CALLBACK_NOT_REGISTERED = -5004,

	// return codes for Main Image Buffer management
	BUFFER_UNINITIALIZED = -5020,
	BUFFER_ALLOCATION_FAILURE = -5021,
	BUFFER_IS_EMPTY = -5022,
	BUFFER_BYTE_ALLIGNMENT_ERROR = -5023,
	DATA_RATE_MEASUREMENT_FAILURE = -5024,
	BUFFER_TOO_SMALL = -5025,
	BUFFER_BUSY = -5026,

	// return codes for file management
	FILE_INCORRECT_TYPE = -5040,
	FILE_IDENTIFIER_NOT_READ = -5041,
	FILE_OPEN_FAILURE = -5042,
	FILE_CREATION_FAILURE = -5043,
	FILE_WRITING_FAILURE = -5044,
	FILE_PATHNAME_TOO_LONG = -5045,
	NOT_ALLOWED_WHILE_IMAGING = -5046,
	DATA_TYPE_NOT_SUPPORTED = -5047,
	FILE_READING_FAILURE = -5048,

	// return codes for JPEG compression/decompression and BMP export
	JPEG_LOAD_LIBRARY_FAILED = -5050,
	JPEG_LOAD_FUNCTION_FAILED = -5051,
	JPEG_INIT_FAILED = -5052,
	JPEG_DECOMPRESSION_ERROR = -5053,
	JPEG_COMPRESSION_ERROR = -5054,
	BMP_WRITING_FAILURE = -5055,
	OPENCV_LOAD_LIBRARY_FAILED = -5056,
	IMAGE_EXPORT_RESOURCES_MISSING = -5057,

	// return codes for analog waveform generation control (axScanCmd() and related functions)
	DAQMX_LOAD_LIBRARY_FAILED = -9000,
	DAQMX_LOAD_FUNCTION_FAILED = -9001,
	DAQMX_GET_INFO_FAILED = -9002,
	DAQMX_RESET_FAILED = -9003,
	DAQMX_TASK_CREATION_FAILED = -9004,
	DAQMX_CONFIGURATION_FAILED = -9005,
	DAQMX_INVALID_SCAN_PARAMS = -9006,
	DAQMX_MAX_VOLTAGE_EXCEEDED = -9007,
	//RFU1 = -9008,
	DAQMX_COMMAND_DEPRECATED = -9009,
	DAQMX_BURST_RASTER_NOT_SETUP = -9010,
	DAQMX_COMMAND_NOT_RECOGNIZED = -9011,
	DAQMX_WAIT_TIMEOUT = -9012,
	DAQMX_MUTEX_TIMEOUT = -9013,
	DAQMX_UNSUPPORTED_DEVICE = -9014,
	DAQMX_DEVICE_NOT_FOUND = -9015,
	DAQMX_ALREADY_DESTROYED = -9016,

	// return codes for OpenGL display (axSetupDisplay() and related functions)
	RENDER_MODULE_HANDLE_UNAVAILABLE = -8000,
	RENDER_WINDOW_CLASS_REGISTRATION_FAILED = -8001,
	RENDER_WINDOW_CREATION_FAILED = -8002,
	RENDER_DEVICE_CONTEXT_CREATION_FAILED = -8003,
	RENDER_PIXEL_SETUP_FAILED = -8004,
	RENDER_CONTEXT_CREATION_FAILED = -8005,
	RENDER_GLEW_INIT_FAILED = -8006,
	RENDER_CONTEXT_CURRENT_FAILURE = -8007,
	RENDER_OPENGL_VERSION_UNSUPPORTED = -8008,
	RENDER_SHADER_PROGRAM_FAILED = -8009,
	RENDER_WAITING_FOR_THREAD = -8010,
	RENDER_THREAD_NOT_RESPONDING = -8011,
	RENDER_OPENGL_FUNCTION_ERROR = -8012,
	RENDER_FUNCTIONS_NOT_SETUP = -8013,
	RENDER_WINDOW_MOVE_FAILED = -8014,
	RENDER_WINDOW_STYLE_FAILED = -8015,
	RENDER_WINDOW_NOT_SETUP = -8016,

	// return codes for Ethernet packet capture
	PCAP_LOAD_LIBRARY_FAILED = -7000,
	PCAP_LOAD_FUNCTION_FAILED = -7001,
	PCAP_NO_DEVICES_FOUND = -7002,
	PCAP_INTERNAL_ERROR = -7003,
	PCAP_DEVICE_OPEN_FAILURE = -7004,
	PCAP_DEVICE_SETUP_FAILURE = -7005,

	// return codes for PCIe packet capture
	PCIE_LOAD_LIBRARY_FAILED = -7020,
	PCIE_LOAD_FUNCTION_FAILED = -7021,
	PCIE_INTERFACE_NOT_SETUP = -7022,
	PCIE_DEVICE_SETUP_FAILURE = -7023,
	PCIE_DRIVER_SETUP_FAILURE = -7024,
	PCIE_DAQ_NOT_INSTALLED = -7025,
	PCIE_DAQ_INFO_UNAVAILABLE = -7026,
	PCIE_DEVICE_OPEN_FAILURE = -7027,
	PCIE_BUFFER_LOCK_FAILURE = -7028,
	PCIE_WRITE_REGISTER_FAILURE = -7029,
	PCIE_READ_REGISTER_FAILURE = -7030,
	PCIE_SYNC_IO_FAILURE = -7031,
	PCIE_INTERRUPT_ENABLE_FAILURE = -7032,
	AQDAQ_LOAD_LIBRARY_FAILED = -7033,
	AQDAQ_LOAD_FUNCTION_FAILED = -7034,
	AQDAQ_FUNCTION_ERROR = -7035,
	AQDAQ_NO_IMAGE_AVAILABLE = -7036,

	// return codes for image/frame/packet request functions
	DATA_INDEX_INVALID = -9991,
	DATA_NOT_COMPRESSED = -9992,
	DATA_TYPE_OR_LENGTH_CHANGED = -9993,
	DATA_ALLOCATION_TOO_SMALL = -9994,
	DATA_INCOMPLETE_OR_MISSING = -9996,
	DATA_RETURNED_IN_PREVIOUS_REQUEST = -9997,
	DATA_NOT_FOUND_IN_BUFFER = -9999,

	UNINITIALIZED = -10001,             
	CANCELLED_BY_USER = -10002,			
	COMMAND_NOT_IMPLEMENTED = -10003,	
	ALREADY_INITIALIZED = -10005,		
	INSUFFICIENT_ALLOCATION = -10006,	
	DATA_VALIDATION_FAILED = -10008,	
	COMMUNICATION_ERROR = -10010,		

	COMMAND_NOT_RECOGNIZED = -55,		
	USB_CHECKSUM_ERROR = -104,			
	PAYLOAD_CHECKSUM_ERROR = -108,		
	CANNOT_START_TEC_IN_ERROR = -126,	
	INVALID_CONFIG_PARAMS = -138,		
	CONFIG_NOT_SETUP = -140,			

	USB_READ_ERROR = -1000,				
	USB_WRITE_ERROR = -1001,			
	NO_DEVICE_PRESENT = -1002,			

	TCP_READ_ERROR = -1020,				
	TCP_WRITE_ERROR = -1021,			
	TCP_CONNECT_ERROR = -1022,			

	SERIAL_READ_ERROR = -1025,			
	SERIAL_WRITE_ERROR = -10250,		

	DATA_SIZE_TOO_BIG = -1029,			
	HOSTSIDE_USB_ERROR = -1033,			
	USB_COMMUNICATION_ERROR = -1038,	
	MUTEX_TIMEOUT = -1039,				
	DAC_TABLE_NOT_LOADED = -1048, 
	USB_LOAD_LIBRARY_FAILED = -10011,
	USB_LOAD_FUNCTION_FAILED = -10012

} AxErr;


/*! \brief DAQ pipeline modes. */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxPipelineMode {
	UNKNOWN = 0,			/*!< Undefined mode (invalid value in FPGA Register 61?).  */
	RAW_ADC = 1,			/*!< Sampled data directly from ADC (unprocessed). */
	WINDOWED = 2,			/*!< After windowing and zero-padding. */
	IFFT = 3,				/*!< After calculation of Fourier transformation and negative frequency truncation. */
	MOD_SQUARED = 4,		/*!< After calculation of complex modulus squared. */
	SQRT = 5,				/*!< After calculation of square root and background subtraction. */
	LOG = 6,				/*!< After logarithmic compression. */
	EIGHT_BIT = 7,			/*!< After dynamic-range compression to 8-bits. */
	JPEG_COMP = 8,			/*!< After JPEG compression (Ethernet interface only). */
	PIPELINEMODE_RFU = 9	/*!< Reserved for future use. */
} AxPipelineMode;

/*! \brief DAQ channel selection modes. */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxChannelMode {
	CHAN_1 = 0,					/*!< Channel 1 only. */
	CHAN_2 = 1,					/*!< Channel 2 only. */
	SUM_CHANNELS = 2,			/*!< Channels 1 and 2 mixed using vector sum (e.g. polarization diverse). */
	INTERLEAVE_CHANNELS = 3,	/*!< Channels 1 and 2 interleaved. */
	CHANNELMODE_RFU = 4			/*!< Reserved for future use. */
} AxChannelMode;

/** \brief Integer data types which may be generated by the Axsun DAQ depending on selected pipeline bypass mode. */
typedef enum
#ifdef __cplusplus
class
#endif // __cplusplus
AxDataType {
	/** Unsigned 8-bit. */
	U8,
	/** Unsigned 16-bit, big endian. */
	U16,
	/** Unsigned 32-bit, big endian. */
	U32,
	/** 32-bit complex (Signed 16-bit imaginary, Signed 16-bit real), big endian. */
	CMPLX,
	/** Signed 16-bit, big endian. */
	I16,
	/** Unsigned 16-bit, little endian. */
	U16_le,
	/** Unsigned 32-bit, little endian. */
	U32_le,
	/** 32-bit complex (Signed 16-bit imaginary, Signed 16-bit real), little endian. */
	CMPLX_le,
	/** Signed 16-bit, little endian. */
	I16_le,
	/** Signed 32-bit, little endian. */
	I32_le,
	/** (reserved for future use). */
	AXDATATYPE_RFU
} AxDataType;


#endif	//AXSUNCOMMONENUMS_H include guard
