// AxsunOCTCapture.h 
// Copyright 2020 Axsun Technologies
/** \file AxsunOCTCapture.h
* \brief This header file contains all exported function prototypes, structures, and enums necessary for integrating _AxsunOCTCapture.dll_ into a client application.

Please note that **SIGNIFICANT API CHANGES** have been made in this version 3.x relative to previous 2.x versions of the library:

General Functionality:
- defined new enum for Axsun error codes: \link AxErr \endlink.  **NOTE change in return type definition and error code values!**
- adopted opaque handle scheme (\link AOChandle \endlink)
- axStartSession() buffer capacity argument in units of MB (instead of packets)
- axSelectInterface() function required to select Ethernet or PCIe capture interface following axStartSession()
- added axGetErrorString() function to provide verbose description of error codes of type \link AxErr \endlink
- run-time error (instead of load-time error) reporting for when dependencies are not installed correctly
- added binary, JPEG, and BMP image export functionality using axExportImages()
- added ability to resize Main Image Buffer using axResizeBuffer()

DAQmx Scanner Control Functionality:
- redefined error reporting in axScanCmd().  **NOTE change in return type definition and error code values!**
- axScanCmd() now supports multiple connected scanner devices with `GET_DEVICE_SERIALNO` and `SELECT_DEVICE` commands
- scanner devices can be connected/disconnected/reconnected at any time; pre-connection is no longer required
- ability to register scanner device connect/disconnect callback functions for both C-based and LabVIEW clients with axRegisterScannerConnectCallback()

OpenGL Display Functionality:
- redefined error reporting in axSetupDisplay() and all related OpenGL display functions.  **NOTE change in return type definition and error code values!**
- added axUpdateWindowStyle() to change the OpenGL display window style (fixed vs. floating) after initial window creation
- OpenGL interpolation behavior now controlled by a separate function axSelectInterpolation(), rather than axScanConvert()


DETAILED RELEASE NOTES

Version 3.0.0.0
- Initial release of v3.

Version 3.1.0.0
- Added square wave synthesizer and general purpose digital output to NI USB-6211 DAQ card via axScanCmd(...).

Version 3.2.0.0
- Added support for OpenGL rendering of U16, I16, and U32 data types (CMPLX data type not supported).
- Added auto-scale functionality for Brightness and Contrast on OpenGL-rendered images via axAdjustBrightnessContrast().
- Added advanced image export functionality in axExportImagesAdv(), including cropping, dimensional scaling, and video export.
- Added callback function support for image export status in axExportImages().
- Added support for sinusoidal waveform and ramp waveform with arbitrary symmetry in the X dimension via axScanCmd().
- Added framework for third-party DAQ devices (experimental/unsupported).

Version 3.3.0.0
- Redefine axRequestImageAdv() to incorporate setup functionality previously provided in axDownsampling() and axImageRequestSize() via the new struct request_prefs_t.
- Added callback function support for transitions in DAQ imaging mode; see axRegisterDAQCallback() and axReportImagingState().
- Consolidated functionality of axClearBuffer() and axResizeBuffer() into single function, axResizeBuffer().
- Added support for Ch 1 & Ch 2 interleaving (FGPA version 6.70.0.x and later).
- Added support for little-endian formatted data; see AxDataType struct (experimental/unsupported).
- axPipelineMode() function updated to also control channel selection (Ch 1, Ch 2, vector sum, or interleaved).

Version 3.3.1.0
- updated WinDriver dependency to version 14.4.0.

Version 3.4.0.0
- use FPGA version 6.70.0.x or later for full functionality.
- axGetImageInfo() and axRequestImage() now pass a pointer to image_info_t, consolidating the retrieved image information into a single struct. 
- axGetImageInfoAdv() DEPRECATED and may be removed in a future version. New applications should use axGetImageInfo().
- axRequestImageAdv() and axDisplayImage() DEPRECATED and may be removed in a future version. New applications should use axRequestImage().
- changed definition of axpacket_t structure (for internal use).

Version 3.4.1.0
- fixed bug related to unreliable warning of stale images (AxErr = -9997) in axGetImageInfo().

Version 3.4.2.0
- improved PCIe imaging/recording controls and status indication via callback or axGetStatus().
- added new functionality to axExportImagesAdv() via export_prefs_t argument.
- corrected typographical error in AxColormap typedef enum.
- supports installation of current and prior versions of the PCIe device driver (1440, 1421, 1290).

Version 3.4.3.0
- use FPGA version 6.70.0.11 or later for full functionality.
- added items to AxDAQEvent enum to indicate completion of a Burst Record operation and start/stop of load-from-disk operation.
- axGetImageInfo() takes requested_image parameter as uint32_t instead of int64_t; use 0 instead of -1 for the most recent image.

Version 3.4.4.0
- improved debug logging functionality (for internal use).
- fix error in axGetDataRate() when total packet count exceeds 2^31

Version 3.4.5.0
- requires FPGA version 6.70.1.X or later
- support for little endian image data
- support for little endian header metadata
- support for NI USB-6343 DAQ card including auxiliary DC analog output via axScanCmd(SET_AUX_DC_ANALOG_OUTPUT, ...)
- replaced OS's "timeval" struct with custom "axtimeval_t" for cross-platform consistency

Version 3.5.0.0
- added callback functionality for arrival of new images into Main Image Buffer; see AxRegisterNewImageCallback()
- axScanCmd() support for NI USB-6001 and USB-6008 DAQ cards (Digital out and DC analog out only)
- fixed error impacting estimated data rate calculation for PCIe interface
-

*/

#ifndef AXSUNOCTCAPTURE_H
#define AXSUNOCTCAPTURE_H

// includes
#include "AxsunCommonEnums.h"

// defines
constexpr auto MOST_RECENT_IMAGE = uint32_t{ 0 };	 // safer than a #define preprocessor macro

/** \brief Opaque structure used to manage created capture sessions. */
struct CaptureSession;

/** \brief Axsun OCT Capture handle - a pointer to the opaque structure used to manage created capture sessions. */
typedef struct CaptureSession* AOChandle;

/** \brief Dimension for sectioning data cube with axRequestCutPlane(). */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxDimension {
	/** Enface. */
	XY,
	/** Primary (fast) B-scan dimension. */
	XZ,
	/** Secondary (slow) B-scan dimension. */
	YZ
} AxDimension;

/** \brief Available colormaps to select using axSelectColormap(). */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxColormap {
	SEPIA,
	GREYSCALE,
	INV_GREYSCALE,
	USER_COLORMAP
} AxColormap;

/** \brief Available file types to export using axExportImages() and axExportImagesAdv(). */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxFileType {
	/** Raw binary. */
	BINARY,
	/** JPEG. */
	JPEG,
	/** Windows Bitmap. */
	BMP,
	/** PNG. */
	PNG,
	/** (reserved for future use). */
	FILETYPE_RFU
} AxFileType;

/** \brief Available request modes to define behavior of axRequestImageAdv(). */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxRequestMode {
	/** Copy image into the user-provided preallocated buffer. */
	RETRIEVE_TO_CALLER,
	/** Display the image in an OpenGL window. */
	DISPLAY_ONLY,
	/** Copy image into the user-provided preallocated buffer and display the image in an OpenGL window. */
	RETRIEVE_AND_DISPLAY,
	/** (reserved for future use). */
	REQUESTMODE_RFU
} AxRequestMode;

/** \brief Types of transitions in DAQ's operational mode, conveyed via user callback function. See axRegisterDAQCallback() for usage. */
typedef enum
#ifdef __cplusplus
class
#endif // __cplusplus
AxDAQEvent {
	/** Image data stopped being enqueued into the main image buffer. */
	IMAGING_STOP = 0,
	/** Image data started being enqueued into the main image buffer (via DAQ Live, DAQ Record, or via loading from disk). */
	IMAGING_START = 1,
	/** Image data stopped being enqueued into the main image buffer following Burst Record mode. */
	RECORD_DONE = 2,
	/** Image data started to be enqueued via Burst Record mode. */
	RECORD_START = 3,
	/** DAQ completed calibration routine. */
	CALIBRATION_DONE = 4,
	/** DAQ started calibration routine. */
	CALIBRATION_START = 5,
	/** Image data stopped being enqueued into the main image buffer following loading from disk. */
	LOAD_DONE = 6,
	/** Image data started to be enqueued via loading from disk. */
	LOAD_START = 7
} AxDAQEvent;

/** \brief Available capture interfaces to be selected by axSelectInterface(). */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxInterface {
	/** Gigabit Ethernet interface. */
	GIGABIT_ETHERNET,
	/** PCI Express x8 interface. (WINDOWS ONLY) */
	PCI_EXPRESS,
	/** No interface selected. */
	NO_INTERFACE,
	/** (reserved for future use). */
	INTERFACE_RFU
} AxInterface;

/** \brief Available scanner command types to be executed by axScanCmd(). */
typedef enum 
#ifdef __cplusplus
class
#endif // __cplusplus
AxScannerCommand {
	/** Initialize & allocate scanner control resources (DEPRECATED). */
	INIT_SCAN,
	/** Destroy & deallocate scanner control resources. */
	DESTROY_SCAN,		
	/** Configure a raster scan pattern with linear ramp waveforms on both X and Y dimensions. */
	SET_RECT_PATTERN,			
	/** Load an externally generated scan pattern. */
	LOAD_EXT_PATTERN,			
	/** Start continuous line scanning based on configured scan pattern. */
	CONTINUOUS_LINE_SCAN,	 
	/** Start continuous raster scanning based on configured scan pattern. */
	CONTINUOUS_RASTER_SCAN,	
	/** Move to a configured position and stop scanning. */
	STOP_AT_POSITION,			
	/** Prepare burst raster scan by pre-loading buffers and waiting at start position. */
	SETUP_BURST_RASTER,	
	/** Start burst raster scan with minimum latency. */
	START_BURST_RASTER,
	/** Wait for burst raster to complete. */
	WAIT_BURST,
	/** Change between external and internal sample clock. */
	SET_SAMPLE_CLOCK,
	/** Get the device serial number. */
	GET_DEVICE_SERIALNO,
	/** Select a device to be the active device. */
	SELECT_DEVICE,
	/** Count number of connected devices. */
	COUNT_DEVICES,
	/** Set maximum allowable voltage for X scan and Y scan waveform outputs. */
	SET_MAX_VOLTAGE,
	/** Control general purpose digital output. */
	SET_DIGITAL_OUTPUT,
	/** Control general purpose square wave synthesizer. */
	SET_SQUAREWAVE_OUTPUT,
	/** Configure a raster scan pattern with sinusoidal waveform on the X dimension and linear ramp waveform on the Y dimension. */
	SET_XSINE_YRAMP_PATTERN,
	/** Control general purpose DC analog output voltage. */
	SET_AUX_DC_ANALOG_OUTPUT,
	/** Set general purpose DC analog output voltage maximum limit. */
	SET_AUX_DC_ANALOG_MAX,
	/** Set general purpose DC analog output voltage minimum limit. */
	SET_AUX_DC_ANALOG_MIN
} AxScannerCommand;

/** \brief Structure for passing raster scan pattern parameters to axScanCmd(SET_RECT_PATTERN, ...). */
typedef struct scan_params_t {	// for analog waveform generation control
	/** The number of increments in the X dimension (i.e. number of A-scans per B-scan). Must be an even value between 2 and 10000. (Note values less than 256 will exceed the max Image_sync pulse frequency for a system running at 100kHz A-line rate.) */
	uint32_t X_increments;
	/** The number of increments in the Y dimension (i.e. number of B-scans per volume scan). Must be an even value between 2 and 10000. */
	uint32_t Y_increments;
	/** The peak output voltage in the X dimension (impacts lateral length of each B-scan). Negate this value to flip the B-scan orientation. Generated voltages will span -X_range to +X_range, centered at the origin defined by X_shift.*/
	double X_range;
	/** The peak output voltage in the Y dimension (impacts lateral length across all B-scans). Negate this value to flip the volume scan orientation. Generated voltages will span -Y_range to +Y_range, centered at the origin defined by Y_shift.*/
	double Y_range;
	/** The voltage to shift the origin in the X dimension (can be positive or negative, 0 = centered). */
	double X_shift;
	/** The voltage to shift the origin in the Y dimension (can be positive or negative, 0 = centered). */
	double Y_shift;
	/** The static voltage generated in the Y dimension during 1D/linear scanning (can be positive or negative, relative to the origin defined by Y_shift). Ignored for 2D/raster scanning.*/
	double Y_idle;
	/** A phase shift applied to the X dimension waveform to control the relative delay between the analog output waveform and the Image_sync pulse. Expressed as a percentage on the interval of 0 to 100; values outside this range will be coerced. */
	double X_phase;
	/** Symmetry of the linear ramp waveform in the X dimension. Expressed as a percentage on the interval of 0 to 100; values outside this range will be coerced. A value of 50 generates a symmetric triangle wave. A value of 0 or 100 generates a sawtooth wave. Ignored when X dimension configured for sine waveform.*/
	double X_symmetry;
} scan_params_t;

/** \brief Structure for passing a user-generated scan pattern to axScanCmd(LOAD_EXT_PATTERN, ...). */
typedef struct ext_pattern_t {	// for analog waveform generation control
	/** The number of increments in the X dimension (i.e. number of A-scans per B-scan). Must be an even value between 2 and 10000. This field also defines the Image_sync pulse frequency which is constant for the entire raster scan. (Note values less than 256 will exceed the max Image_sync pulse frequency for a system running at 100kHz A-line rate.)*/
	uint32_t ext_X_increments;
	/** The number of increments in the Y dimension (i.e. number of B-scans per volume scan). Must be an even value between 2 and 10000. */
	uint32_t ext_Y_increments;
	/** Pointer to array definining the linear (1D) scan pattern. Length of array must be 2 * ext_X_increments (X and Y voltages interleaved for each output sample).*/
	double * linear_pattern;
	/** Pointer to array definining the raster (2D) scan pattern. Length of array must be 2 * ext_X_increments * ext_Y_increments (X and Y voltages interleaved for each output sample).*/
	double * raster_pattern;
} ext_pattern_t;


/** \brief Structure for image request preferences. */
typedef struct request_prefs_t {
	
	/** Indicate if requested image is to be retrieved to the caller, displayed directly via OpenGL, or both. */
	AxRequestMode request_mode;
	/** Selects the OpenGL window number targeted for image display (if request_mode is DISPLAY_ONLY or RETRIEVE_AND_DISPLAY). */
	int32_t which_window; 
	/** Consecutive images to average (mean) up to maximum of 10. Set to 1 for no image averaging. Image width must be <= 5000 A-scans. */
	uint32_t average_number;
	
	/** Cropping an image along the A-scan dimension (width) avoids inefficiency associated with retrieving unwanted A-scans which would subsequently be discarded by the calling application before display (e.g. galvo fly-back portion).
	
	Cropping is not available in Force Trigger mode (i.e. when no Image_sync is detected).
	
	Note that the cropping behavior based on these settings is applied prior to the OpenGL display window cropping behavior configured using axCropRect().

	The first A-scan to be retrieved, a positive-valued offset from the Image_sync pulse defining the start of an image. If the offset exceeds the available A-scans in an image, the requested image will be retrieved/displayed starting at offset = 0.*/
	uint32_t crop_width_offset;
	/** The total number of A-scans to be retrieved. Set to 0 to retrieve the full image. If the value exceeds the remaining A-scans available following crop_width_offset, the remaining available A-scans in the image will be retrieved/displayed. */
	uint32_t crop_width_total;
	/** Placeholder for future functionality. */
	uint32_t crop_height_offset;
	/** Placeholder for future functionality. */
	uint32_t crop_height_total;
	/** Placeholder for future functionality. Controls 16-bit to 8-bit dynamic range compression behavior. */
	int16_t to8bit_shift;
	/** Placeholder for future functionality. Controls 16-bit to 8-bit dynamic range compression behavior. */
	uint16_t to8Bit_min;
	/** The desired 2x downsampling behavior (disabled = 0, enabled = 1). Downsampling speeds up JPEG decompression but halves the width and height of retrieved and/or displayed images. The original full-resolution images persist in the Main Image Buffer. Downsampling is helpful to keep frame rates high during display of images that have very high numbers of a-scans per image and are thus larger than the display's pixel dimensions. */
	uint8_t downsample;
	/** Placeholder for future functionality. */
	uint8_t dezipper;
} request_prefs_t;


/** \brief Structure for image export preferences (movie functionality requires installation of OpenCV). */
typedef struct export_prefs_t {
	/** Width of the exported image in pixels. If = 0, the exported width will be that of the requested image.*/
	uint32_t width;
	/** Height of the exported image in pixels. If = 0, the exported height will be that of the requested image.*/
	uint32_t height;
	/** Control the contrast (gain) of the exported image.  A value of 1 results in no change. */
	double contrast;
	/** Control the brightness (offset) of the exported image.  A value of 0 results in no change. */
	double brightness;
	/** Selects a colormap for the exported image. Supports images with U8 data type only. */
	AxColormap colormap;
	/** Selects a colormap from the available OpenCV colormaps listed at https://docs.opencv.org/4.4.0/d3/d50/group__imgproc__colormap.html, only if "colormap" parameter is AxColormap::USER_COLORMAP. */
	int32_t opencv_colormap;
	/** The number of frames per second for the exported movie file. If < 1, movie file export is disabled. Movie files can be exported only for U8 data type. "width" and "height" parameters must be non-zero for movie export.*/
	int32_t movie_fps;
} export_prefs_t;

/** \brief Structure for packet metadata (for internal use.) */
typedef struct axpacket_t {
	uint32_t image_number;
	uint32_t frame_number;
	uint32_t packet_number;
	uint16_t ascan_length;
	uint16_t data_type_code;
	uint16_t status_flags;
	uint16_t frames_packet_number;
	uint16_t sync_offset;
	uint8_t subsamp;
	uint8_t RFU;
	struct axtimeval_t time_stamp;
} axpacket_t;

/** \brief Structure for conveying metadata information about an image. */
typedef struct image_info_t {
	/** Unique image number.  A value of zero indicates a force-triggered image.*/
	uint32_t image_number;
	/** Height (pixels). */
	int32_t height;
	/** Width (pixels). */
	int32_t width;
	/** Total image size (bytes): the required size of a buffer allocated by the user for image retrieval using axRequestImageAdv() or axRequestImage()*/
	uint32_t size_bytes;
	/** Data type (signed or unsigned, 8-, 16-, or 32-bits, big endian or little endian, etc. */
	AxDataType data_type;
	/** Pipeline mode (raw data, windowed, log compressed, JPEG compressed, etc. */
	AxPipelineMode pipeline_mode;
	/** Channel selected (single channel, vector sum, interleaved channels. */
	AxChannelMode channel_mode;
	/** Image acquisition time stamp in UNIX Epoch time (seconds since 00:00:00 UTC on 1 Jan 1970). */
	struct axtimeval_t time_stamp;
	/** The subsampling factor used during capture of this image. */
	uint8_t subsampling_factor;
	/** Dynamic range adjustment for conversion of pixel values into physical units (e.g. volts). */
	uint8_t shift_adj;
	/** If non-zero, image_sync trigger is too slow; image was force-triggered. */
	uint8_t force_trig;
	/** If non-zero, image_sync trigger is too fast; image width possibly affected by missed trigger. */
	uint8_t trig_too_fast;
	/** If non-zero, this image was captured during a Burst Record operation. */
	uint8_t burst_recorded;
	/** If non-zero, this image's data source is FPGA test vectors, not sampled ADC data. */
	uint8_t test_vector_data;
	/** Packet metadata for the last packet in this image (for internal use). */
	axpacket_t last_packet_metadata;
} image_info_t;

/** \brief Structure for data made available to user in a new image callback event via registered AxNewImageCallbackFunction_t. */
typedef struct new_image_callback_data_t {
	/** The current AxsunOCTCapture session handle. */
	AOChandle session;
	/** The unique number of the newly enqueued image responsible for triggering the callback, or 0 if in force-triggered mode. */
	uint32_t image_number;	
	/** The size (in bytes) of a buffer that must be allocated before image retrieval using axRequestImage(). */
	uint32_t required_buffer_size;
};

/** \brief A user-provided function to be called when a new image is enqueued in the Main Image Buffer. See axRegisterNewImageCallback() for usage.*/
typedef void(__cdecl* AxNewImageCallbackFunction_t)(new_image_callback_data_t, void*);

/** \brief A user-provided function to be called upon connection or disconnection of a NI-DAQmx device. See axRegisterScannerConnectCallback() for usage.*/
typedef void(__cdecl* AxConnectScannerCallbackFunction_t)(void*);

/** \brief A generic user-provided callback function with an unsigned int and void* argument. See axRegisterExportCallback(), or axRegisterDAQCallback() for usage.*/
typedef void(__cdecl* AxCallbackFunction_t)(uint32_t, void*);


#ifdef __cplusplus
extern "C" {
#endif

	/**
	\brief Start an Axsun DAQ imaging session by allocating memory for the Main Image Buffer.
	\param session Pointer to session's opaque handle which will be created by this function and used as an argument for all subsequent function calls specific to this session.
	\param capacity_MB The desired size (in megabytes) to allocate for the Main Image Buffer.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axStartSession() is the first method called in a typical implementation of the AxsunOCTCapture API.
	Sessions started with axStartSession() must be closed with axStopSession() before the client application exits or else resources will be left allocated.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axStartSession(AOChandle* session, float capacity_MB);

	/**
	\brief Select the data interface (Ethernet, PCIe, or none) for the capture session.
	\param session The opaque handle to the session created with axStartSession().
	\param which_interface The desired interface type to initialize for this session.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details This function will replace/reset an existing interface which was already setup from a previous call to this function.  A capture session can only have one associated interface at a time.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axSelectInterface(AOChandle session, AxInterface which_interface);

	/**
	\brief Stop a capture session and deallocate all resources, including Main Image Buffer and interfaces.
	\param session The opaque handle to the session created with axStartSession().
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axStopSession() is the last function called in a typical implementation using the AxsunOCTCapture API. A session started with axStartSession() must be closed with axStopSession() before the client application exits.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */		
		AxErr axStopSession(AOChandle session);

	/**
	\brief Get a description of a specific AxErr error code.
	\param errorcode An AxErr error code returned from another function in the AxsunOCTCapture API.
	\param message_out A pointer to a pre-allocated output buffer of characters with size at least 512 bytes into which a nul-terminated description for errorcode will be copied.
	\details axGetErrorString() can be called at any time; creation of a valid capture session is not required.
	It is unsafe to pass a message_out output buffer allocated with fewer than 512 bytes.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		void axGetErrorString(AxErr errorcode, char* message_out);

	/**
	\brief Get a description of the capture session's interface status.
	\param session The opaque handle to the session created with axStartSession().
	\param message_out A pointer to a pre-allocated buffer of characters with size at least 256 bytes into which a nul-terminated message string will be copied.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axGetMessage() can be called any time after creation of a valid capture session and will describe the status of the capture interface.  A capture session without a valid capture interface will support only review of previously saved and re-loaded data using axSaveFile() and axLoadFile().
	It is unsafe to pass an output buffer allocated with fewer than 256 bytes.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetMessage(AOChandle session, char* message_out);

	/**
	\brief Get imaging mode status and Main Image Buffer statistics.
	\param session The opaque handle to the session created with axStartSession().
	\param imaging Interpreted as a bit-field: bit 0 set indicates imaging is active, bit 1 set indicates burst recording is active, bit 2 set indicates loading from disk is active.
	\param last_packet_in Will be populated with the unique packet number most recently enqueued into the Main Image Buffer.
	\param last_frame_in Will be populated with the unique frame number most recently enqueued into the Main Image Buffer.
	\param last_image_in Will be populated with the unique image number most recently enqueued into the Main Image Buffer.
	\param droppedPackets Will be populated with the number of packets dropped since the last imaging mode reset.
	\param frames_since_sync Will be populated with the number of frames enqueued since the last Image_sync pulse was received.  When this number reaches the configured trigger timeout, the driver will transition to Force Trigger mode.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetStatus(AOChandle session, uint32_t * imaging, uint32_t * last_packet_in, uint32_t * last_frame_in, uint32_t *last_image_in, uint32_t * dropped_packets, uint32_t *frames_since_sync);

	/**
	\brief Get data transfer rate on the active interface.
	\param session The opaque handle to the session created with axStartSession().
	\param datarate_Mbps The estimated data transfer rate for the most recently enqueued image data in in megabits/second.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/	
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetDataRate(AOChandle session, double * datarate_Mbps);

	/**
	\brief Get the rate of new images being enqueued in the main image buffer.
	\param session The opaque handle to the session created with axStartSession().
	\param images_per_second The estimated rate for the most recently enqueued image data in images/second.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axGetImageRate(AOChandle session, int32_t * images_per_second);

	/**
	\brief Get information on an image in the Main Image Buffer.
	\param session The opaque handle to the session created with axStartSession().
	\param requested_image The image number for which information is desired. This can be a unique image number > 1, or it can be 0 to get info on the most recently enqueued image in the buffer.
	\param image_info A image_info_t struct which will be populated with information about the requested image.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axGetImageInfo() gets information on an image in the Main Image Buffer before a more computationally expensive request is made to retrieve or display the image.
	A call to axGetImageInfo() is intended to precede a call to axRequestImage() and the returned image_info provides several arguments to that subsequent function call (e.g. output_buf_len, requested_image) and identifies "stale" images (for avoiding the wasted computation resources incurred by calling axRequestImage() using the same requested image number on consecutive calls).
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetImageInfo(AOChandle session, uint32_t requested_image, image_info_t* image_info);

	/**
	\brief (DEPRECATED in v3.4.0) Advanced function for getting information on an image in the Main Image Buffer.
	\param session The opaque handle to the session created with axStartSession().
	\param requested_image_number The image number for which information is desired. This can be a unique image number > 1, or it can be set < 1 to get info on the most recently enqueued image in the buffer.
	\param returned_image_number Will be populated with the unique image number. This will be equal to the "requested_image_number" parameter unless that parameter is set < 1 to get info on the most recent image.
	\param height Will be populated with the height of the requested image (in pixels).
	\param width Will be populated with the width of the requested image (in pixels).
	\param data_type_out Will be populated with the data type of the requested image (see AxDataType enum definition)
	\param required_buffer_size Will be populated with the required size (in bytes) of a user buffer that must be allocated before image retrieval using axRequestImageAdv or axRequestImage.
	\param force_trig Will be populated with 1 if requested image was acquired in Force Trigger mode, or 0 otherwise.  See description associated with axSetTrigTimeout function.
	\param trig_too_fast Will be populated with 1 if Image_sync trigger period is too short, or 0 otherwise.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axGetImageInfoAdv() gets information on an image in the Main Image Buffer before a more computationally expensive request is made to retrieve or display the image.
	A call to axGetImageInfoAdv() is intended to precede a call to axRequestImageAdv() and provides several arguments to axRequestImageAdv() (e.g. output_buf_len, requested_image_number) and identifies "stale" images (for avoiding the wasted computation resources incurred by calling axRequestImageAdv() using the same requested image number on consecutive calls).
	This function is DEPRECATED and may be removed in future versions. Use axGetImageInfo() instead.*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetImageInfoAdv(AOChandle session, int64_t requested_image_number, uint32_t * returned_image_number, int32_t * height, int32_t * width, AxDataType *data_type_out, uint32_t * required_buffer_size, uint8_t * force_trig, uint8_t * trig_too_fast);

	/**
	\brief Retrieve and/or display an image from the Main Image Buffer.
	\param session The opaque handle to the session created with axStartSession().
	\param requested_image The image number requested for retrieval or display. This must be a unique image number returned from a prior call to axGetImageInfo(). For the most recently enqueued image in the buffer, use a value < 1 as the requested_image argument in axGetImageInfo(). 
	\param prefs A structure containing preferences related to image averaging, request mode, image cropping, and other behaviors.
	\param output_buf_len The size in bytes of the pre-allocated image_data_out buffer.  Don't lie, or crashes could ensue.
	\param image_data_out A pre-allocated buffer into which the retrieved image is copied for subsequent user interaction. Buffer size must be at least as large as indicated by a preceding call to axGetImageInfo() or axGetImageInfoAdv() (i.e. the required_buffer_size) unless accomodations are made manually for image cropping or downsampling in the request prefs.
	\param image_info An image_info_t struct which will be populated with information about the retreived and/or displayed image.  NOTE: height, width, and size_bytes values may be different from prior call to axGetImageInfo() if the request prefs include cropping or downsampling.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axRequestImage() requests the retrieval (into a pre-allocated buffer defined by the user) or direct display (via OpenGL rendering) of an image from the Main Image Buffer.
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axRequestImage(AOChandle session, uint32_t requested_image, request_prefs_t prefs, uint32_t output_buf_len, uint8_t * image_data_out, image_info_t* image_info);

	/**
	\brief (DEPRECATED in v3.4.0) Advanced function for retrieving and/or displaying an image from the Main Image Buffer.
	\param session The opaque handle to the session created with axStartSession().
	\param requested_image_number The image number requested for retrieval and/or display. This can be a unique image number > 1 or it can be set to -1 to request the most recently enqueued image in the buffer.
	\param image_data_out A pre-allocated buffer into which the retrieved image is copied for subsequent user interaction. Buffer size must be at least as large as indicated by a preceding call to axGetImageInfo or axGetImageInfoAdv ("required_buffer_size"). Can be NULL if req_mode parameter is set to DISPLAY_ONLY.
	\param metadata_out A pre-allocated buffer of sizeof(axpacket_t) bytes into which pertinent image metadata is copied.
	\param height Will be populated with the height of the requested image (in pixels).
	\param width Will be populated with the width of the requested image (in pixels).
	\param data_type_out Will be populated with the data type of the requested image (see AxDataType enum definition)
	\param output_buf_len The pre-allocated length in bytes of the image_data_out buffer. Can be 0 if req_mode parameter is set to DISPLAY_ONLY.
	\param prefs A structure containing preferences related to image averaging, request mode, image cropping, and other behaviors.
	\param force_trig Will be populated with 1 if requested image was acquired in Force Trigger mode, or 0 otherwise.  See description associated with axSetTrigTimeout function.
	\param trig_too_fast Will be populated with 1 if Image_sync trigger period is too short, or 0 otherwise.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axRequestImageAdv() requests the retrieval (into a pre-allocated buffer defined by the user) and/or display (directly rendered into OpenGL window) of an image from the Main Image Buffer.
	A call to axRequestImageAdv() is intended to follow a call to axGetImageInfo(), which provides several arguments to axRequestImageAdv() (output_buf_len, requested_image_number) and identifies "stale" images (for avoiding the wasted computation resources incurred by calling axRequestImageAdv() using the same requested image number on consecutive calls).
	This function is DEPRECATED and may be removed in future versions. Use axRequestImage() or axDisplayImage() instead.*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axRequestImageAdv(AOChandle session, int64_t requested_image_number, uint8_t * image_data_out, axpacket_t * metadata_out, int32_t * height, int32_t * width, AxDataType *data_type_out, uint32_t output_buf_len, request_prefs_t prefs, uint8_t * force_trig, uint8_t * trig_too_fast);

	/**
	\brief Get information on a frame in the Main Image Buffer (A frame is 256 A-scans, unsynchronized with Image_sync signal).
		\param session The opaque handle to the session created with axStartSession().

	\param requested_frame_number The frame number for which information is desired. This can be a unique frame number or it can be set to -1 to get info on the most recently enqueued frame in the buffer.
	\param returned_frame_number Will be populated with the unique frame number. This will be equal to the "requested_frame_number" parameter unless that parameter is set to -1 to get info on the most recent frame.
	\param height Will be populated with the height of the requested frame (in pixels).
	\param width Will be populated with the width of the requested frame (in pixels).
	\param data_type_out Will be populated with the data type of the requested frame (see AxDataType enum definition)
	\param required_buffer_size Will be populated with the required size (in bytes) of a user buffer that must be allocated before frame retrieval using axRequestFrameAdv.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axGetFrameInfoAdv() gets information on a frame in the Main Image Buffer before a more computationally expensive request is made to retrieve the frame data.
	A call to axGetFrameInfoAdv() is intended to precede a call to axRequestFrameAdv() and provides several arguments to axRequestFrameAdv() (e.g. output_buf_len, requested_frame_number) and identifies "stale" images (for avoiding the wasted computation resources incurred by calling axRequestFrameAdv using the same requested frame number on consecutive calls).
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetFrameInfoAdv(AOChandle session, int64_t requested_frame_number, uint32_t * returned_frame_number, int32_t * height, int32_t * width, AxDataType *data_type_out, uint32_t * required_buffer_size);

	/**
	\brief Retrieve a frame from the Main Image Buffer (A frame is 256 A-scans, unsynchronized with Image_sync signal).
	\param session The opaque handle to the session created with axStartSession().
	\param requested_frame_number The frame number requested for retrieval. This can be a unique image number or it can be set to -1 to request the most recently enqueued frame in the buffer.
	\param frame_data_out A pre-allocated buffer into which the retrieved frame is copied for subsequent user interaction. Buffer size must be at least as large as indicated by a preceding call to axGetFrameInfoAdv ("required_buffer_size").
	\param metadata_out A pre-allocated buffer of sizeof(axpacket_t) bytes into which metadata is copied.
	\param height Will be populated with the height of the requested frame (in pixels).
	\param width Will be populated with the width of the requested frame (in pixels).
	\param data_type_out Will be populated with the data type of the requested frame (see AxDataType enum definition)
	\param output_buf_len The pre-allocated length in bytes of the frame_data_out buffer.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axRequestFrameAdv() requests the retrieval (into a pre-allocated buffer defined by the user) of a frame from the Main Image Buffer.
	A call to axRequestFrameAdv() is intended to follow a call to axGetFrameInfoAdv(), which provides several arguments to axRequestFrameAdv() (e.g. output_buf_len, requested_frame_number) and identifies "stale" images (for avoiding the wasted computation resources incurred by calling axRequestFrameAdv using the same requested frame number on consecutive calls).
	OpenGL display of frames is currently unsupported.*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axRequestFrameAdv(AOChandle session, int64_t requested_frame_number, uint8_t * frame_data_out, axpacket_t * metadata_out, int32_t * height, int32_t * width, AxDataType *data_type_out, uint32_t output_buf_len);

	/**
	\brief Get un-decompressed size information on a compressed JPEG frame in the Main Image Buffer (A frame is 256 A-scans, unsynchronized with Image_sync signal).
		\param session The opaque handle to the session created with axStartSession().

	\param requested_frame_number The frame number for which information is desired. This can be a unique frame number or it can be set to -1 to get info on the most recently enqueued frame in the buffer.
	\param returned_frame_number Will be populated with the unique frame number. This will be equal to the "requested_frame_number" parameter unless that parameter is set to -1 to get info on the most recent frame.
	\param required_buffer_size Will be populated with the required size (in bytes) of a user buffer that must be allocated before JPEG retrieval using axRequestCompressedJPEG.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axGetCompressedJPEGInfo() gets information on a JPEG compressed frame in the Main Image Buffer before a request is made to retrieve the JPEG data into a user buffer.
	A call to axGetCompressedJPEGInfo() is intended to precede a call to axRequestCompressedJPEG() and provides several arguments to axRequestCompressedJPEG() (e.g. output_buf_len, requested_frame_number)
	and identifies "stale" images (for avoiding redundant memory copies if calling axRequestCompressedJPEG using the same requested frame number on consecutive calls).
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axGetCompressedJPEGInfo(AOChandle session, int64_t requested_frame_number, uint32_t * returned_frame_number, uint32_t * required_buffer_size);

	/**
	\brief Retrieve an un-decompressed JPEG frame from the Main Image Buffer (A frame is 256 A-scans, unsynchronized with Image_sync signal).
		\param session The opaque handle to the session created with axStartSession().

	\param requested_frame_number The frame number requested for retrieval. This can be a unique image number or it can be set to -1 to request the most recently enqueued frame in the buffer.
	\param JPEG_data_out A pre-allocated buffer into which the retrieved JPEG frame is copied for subsequent user interaction. Buffer size must be at least as large as indicated by a preceding call to axGetCompressedJPEGInfo ("required_buffer_size").
	\param metadata_out A pre-allocated buffer of sizeof(axpacket_t) bytes into which frame metadata is copied.
	\param output_buf_len The pre-allocated length in bytes of the JPEG_data_out buffer.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axRequestCompressedJPEG() requests the retrieval (into a pre-allocated buffer defined by the user) of un-decompressed JPEG information.
	This JPEG can subsequently be saved as a file and/or decompressed by the user's preferred JPEG decompression library or utility.
	A call to axRequestCompressedJPEG() is intended to follow a call to axGetCompressedJPEGInfo(), which provides several arguments to axRequestCompressedJPEG() (e.g. output_buf_len, requested_frame_number)
	and identifies "stale" images (for avoiding redundant memory copies if calling axRequestCompressedJPEG using the same requested frame number on consecutive calls).
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axRequestCompressedJPEG(AOChandle session, int64_t requested_frame_number, uint8_t * JPEG_data_out, axpacket_t * metadata_out, uint32_t output_buf_len);

	/**
	\brief Setup an OpenGL display window for direct rendering of image data. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param window_style The window style: either a floating window with border (=0) or a fixed borderless window (=1).
	\param w_left The initial left edge of the window in display coordinates. Also see axUpdateView().
	\param w_top The initial top edge of the window in display coordinates. Also see axUpdateView().
	\param w_width The initial width of the window. Also see axUpdateView().
	\param w_height The initial height of the window. Also see axUpdateView().
	\param parent_window_handle The window handle (HWND) of an existing window of which the OpenGL window is created as a child window.  Set this to NULL for creating an OpenGL window with no parent.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Any resources allocated with axSetupDisplay() are automatically deallocated by axStopSession() when terminating the Axsun OCT Capture Session. Parameters set the initial window style, size, and location of the OpenGL window at creation but (except the parent/child relationship) these can be subsequently changed according to the axUpdateView() and axUpdateWindowStyle() functions.
	The window is hidden by default after the session is created.  Call axHideWindow() with false argument to show the window after position and style are configured as desired.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axSetupDisplay(AOChandle session, uint8_t window_style, int32_t w_left, int32_t w_top, int32_t w_width, int32_t w_height, uintptr_t parent_window_handle);

	/**
	\brief Count the number of OpenGL display windows currently created. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param num_of_windows Will be populated with the number of OpenGL display windows currently created. This will correspond to the number of times axSetupDisplay() has been successfully called since session creation.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axCountWindows(AOChandle session, int32_t * num_of_windows);

	/**
	\brief Change the OpenGL display window style. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this style change.
	\param window_style The window style: either a floating window with border(= 0) or a fixed borderless window(= 1).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axUpdateWindowStyle(AOChandle session, int32_t which_window,  uint8_t window_style);

	/**
	\brief Change the OpenGL display interpolation mode. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this mode change.
	\param interpolation The desired interpolation mode (bilinear = 0, nearest neighbor = 1).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axSelectInterpolation(AOChandle session, int32_t which_window, uint8_t interpolation);

	/**
	\brief (DEPRECATED in v3.4.0) Display an image from the Main Image Buffer directly to an OpenGL window (8-bit image data only). (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param requested_image_number The image number requested for display. This can be a unique image number or it can be set < 1 to request the most recently enqueued image in the buffer.
	\param returned_image_number Will be populated with the unique image number displayed. This will be equal to the "requested_image_number" parameter unless that parameter is set < 1 to request the most recent image.
	\param height Will be populated with the height of the requested image (in pixels).
	\param width Will be populated with the width of the requested image (in pixels).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axDisplayImage requests the display (directly rendered into an OpenGL window) of an image from the Main Image Buffer. This function is DEPRECATED and may be removed in future versions. Instead use axRequestImage() with the prefs.request_mode member set to DISPLAY_ONLY. */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axDisplayImage(AOChandle session, int64_t requested_image_number, uint32_t * returned_image_number, int32_t * height, int32_t * width);

	/**
	\brief Select the color scheme of images displayed in an OpenGL window. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this style change.
	\param colors The desired color scheme can be SEPIA, grayscale, inverted GREYSCALE, or a user-defined scheme loaded using axLoadUserColormap (see AxColormap enum).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axSelectColormap(AOChandle session, int32_t which_window, AxColormap colors);

	/**
	\brief Load a user-defined AxColormap for images displayed in an OpenGL window. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param user_colormap_in The colormap array to be loaded.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details To display a user-defined color map loaded using this function, the "user" AxColormap must be selected with axSelectColormap().
	The colormap format is a 768-byte array composed of 256 R,G,B triads: (R0, G0, B0), (R1, G1, B1), (R2, G2, B2), ..., (R255, G255, B255).
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axLoadUserColormap(AOChandle session, int32_t which_window, uint8_t * user_colormap_in);

	/**
	\brief Change the polar -> rectangular scan conversion behavior of images displayed in an OpenGL window. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this behavior change.
	\param convert The desired scan conversion behavior (disabled = 0, enabled = 1).
	\param inner_radius A value on the interval [0..1] which defines the inner edge of the annulus onto which an image's r = 0 data is rendered. (0 = center, 1 = outer edge of uncropped display window) 
	\param outer_radius A value on the interval [0..1] which defines the outer edge of the annulus onto which an image's r = Rmax data is rendered. (0 = center, 1 = outer edge of uncropped display window)
	\param crop_inner A value on the interval [-1..1] which defines the fraction of total image cropped prior to scan conversion in the radial direction from r = 0 outward.
	\param crop_outer A value on the interval [0..2] which defines the fraction of total image cropped prior to scan conversion in the radial direction from r = Rmax inward.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Polar-to-rectangular scan conversion is useful for displaying images on a cartesian (x,y) display when acquired with a rotational probe or catheter in polar (r,theta) coordinates.
	For maximized field of view, inner_radius and crop_inner should be set to 0 and outer_radius and crop_outer should be set to 1. 
	Adjusting crop_inner and crop_outer by an equivalent amount will achieve a 'digital Z-offset' radial shifting effect.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axScanConvert(AOChandle session, int32_t which_window, uint8_t convert, float inner_radius, float outer_radius, float crop_inner, float crop_outer);

	/**
	\brief Change the OpenGL window size and position. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this style change.
	\param w_left The left edge of the window in display coordinates (fixed borderless window only).
	\param w_top The top edge of the window in display coordinates (fixed borderless window only).
	\param w_width The width of the window in pixels.
	\param w_height The height of the window in pixels.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Window width and height parameters outside the range [32,4000] will be coerced.
	The w_left and w_top parameters are ignored when the window mode is floating with a border (see axSetupDisplay()).
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axUpdateView(AOChandle session, int32_t which_window, int32_t w_left, int32_t w_top, int32_t w_width, int32_t w_height);

	/**
	\brief Change the rectangular cropping behavior of images displayed in an OpenGL window. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this behavior change.
	\param crop_left Fraction of total image width cropped from left (0 = no cropping, 0.5 = half of image cropped, etc.).
	\param crop_top Fraction of total image height cropped from top.
	\param crop_bottom Fraction of total image height cropped from bottom.
	\param crop_right Fraction of total image width cropped from right.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Cropping parameters less than zero are ignored.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axCropRect(AOChandle session, int32_t which_window, float crop_left, float crop_top, float crop_bottom, float crop_right);

	/**
	\brief Change the brightness and contrast of images displayed in an OpenGL window. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this behavior change.
	\param brightness The desired brightness. Typical values are in the range [-0.5, 0.5].
	\param contrast The desired contrast. Typical values are in the range [0.5, 1.5].
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that if the color scheme is set to INVERTED GREYSCALE (see axLoadColormap), the behavior of the brightness parameter is inverted (i.e. higher values of the brightness parameter result in a darker image).  Setting contrast = 0 auto-scales the Brightness and Contrast values once based on the next rendered image. Setting contrast = -1 auto-scales continuously for all subsequent rendered images.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axAdjustBrightnessContrast(AOChandle session, int32_t which_window, float brightness, float contrast);

	/**
	\brief Hide or unhide the OpenGL image display window. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param which_window Selects which OpenGL window is targeted for this state change.
	\param hidden The desired window visibility state (visible = 0, hidden = 1).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axHideWindow(AOChandle session, int32_t which_window, uint32_t hidden);

	/**
	\brief Control the behavior of Force Trigger mode.
	\param session The opaque handle to the session created with axStartSession().
	\param framesUntilForceTrig The number of frames for which the driver will wait for a Image_sync signal before timing out and entering Force Trigger mode.  Defaults to 24 frames at session creation.  Values outside the range of [2,100] will be automatically coerced into this range.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details The trigger timeout defines the number of consecutive frames for which the driver will wait for a Image_sync signal prior to transitioning into Force Trigger mode.
	Once in Force Trigger mode, the driver will return the most recently captured frame (256 A-scans) with no synchronization with the Image_sync signal (which is either too slow or absent altogether).
	The driver will automatically exit Force Trigger mode and re-synchronize with the Image_sync signal as soon as two consecutive Image_sync signals are detected within the timeout period.
	Set the trigger timeout based on the expected Image_sync signal period of your scanner and the system A-scan rate.
	For example, assume an effective A-scan rate of 100,000 Hz and a Image_sync period of 33 milliseconds (i.e. a B-scan period determined by the frequency of a scanner running at 30 fps).
	The trigger timeout is defined in number of frames (consisting of 256 A-scans each), hence the frame time for a 100kHz system is 2.56 milliseconds (=256/100000).
	A trigger timeout setting of at least 13 frames equaling 33.28 milliseconds (= 2.56 ms * 13) is required to avoid experiencing premature Force Trigger timeout.
	Setting a trigger timeout several frames larger than the minimum is recommended; however, setting it too high will delay or prevent the transition to Force Trigger mode when desired (e.g. when a Image_sync signal is absent).
	Remember that the A-scan subsampling feature on the DAQ reduces the effective A-scan rate of the system and could impact the trigger timeout calculation.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axSetTrigTimeout(AOChandle session, uint32_t framesUntilForceTrig);

	/**
	\brief Save contents of Main Image Buffer to disk.
		\param session The opaque handle to the session created with axStartSession().

	\param path_file Full directory and filename at which to create new save file.
	\param full_buffer Set to 0 for saving only data captured during the most recent imaging sequence (e.g. a burst record) or set to 1 to save the full buffer.
	\param packets_written Will be populated with the number of packets successfully saved to disk.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that a preexisting file at the location indicated for saving will be overwritten without warning. This function should be called only when images are NOT currently being enqueued into the buffer by a connected DAQ board.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axSaveFile(AOChandle session, const char * path_file, uint8_t full_buffer, int32_t * packets_written);

	/**
	\brief Load contents from file on disk into Main Image Buffer.
	\param session The opaque handle to the session created with axStartSession().
	\param path_file Full directory and filename from which to load data.
	\param packets_read Will be populated with the number of packets successfully loaded from disk.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that a subsequent call to axGetStatus is useful for determining the number of frames and images loaded into the Main Image Buffer by this function.  A call to axResizeBuffer() can be used to clear the Main Image Buffer prior to loading.  This function should be called only when images are NOT currently being enqueued into the buffer by a connected DAQ board.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axLoadFile(AOChandle session, const char * path_file, int32_t * packets_read);

	/**
	\brief Clears and resets the Main Image Buffer, optionally with a new buffer size.
	\param session The opaque handle to the session created with axStartSession().
	\param capacity_MB The desired size of the new buffer in MB. A value of 0 resets (clears) the buffer using the previously configured size.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details This function is useful for preventing retrieval of images from prior imaging sequences which have not yet been overwritten by the most recent sequence. Careful management of buffer status (i.e. number of images enqueued) returned using axGetStatus() will avoid this situation, making the use of this function optional. This function should not be called when images are currently being enqueued into the buffer by a connected DAQ board or while loading a file from disk.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axResizeBuffer(AOChandle session, float capacity_MB);

	/**
	\brief Control the image streaming behavior of the Axsun PCIe DAQ between Live Imaging, Burst Recording, and Imaging Off states. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param number_of_images Set this argument to zero (0) for Imaging Off; set it to (-1) for Live Imaging (no record), or set it to any positive value between 1 and 32767 to request the desired number of images in a Burst Record operation.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axImagingCntrlPCIe() controls the DMA-based transfer of images from the Axsun DAQ via the PCIe interface.  Note that this function does NOT control the laser and therefore the laser emission (along with its sweep trigger and k-clocks) must be enabled separately. This function takes a single parameter to select between three states: Live Imaging, Burst Record, or Imaging Off.  In Live Imaging mode, the DAQ will acquire and transmit images indefinitely. In Burst Record mode (a sub-mode of Live Imaging mode), the DAQ will acquire and transmit the finite number of images requested and then automatically transition itself to the Imaging Off state. 
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axImagingCntrlPCIe(AOChandle session, int16_t number_of_images);

	/**
	\brief Write a FPGA register on the Axsun DAQ via the PCIe interface. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param regnum The unique register number to which the write operation is directed.
	\param regval The desired 16-bit value to write.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that this function does not subsequently query the DAQ hardware to confirm the write was actually successful. A return value of NO_AxERROR only indicates the software's request was made successfully. Call axReadFPGAreg() to confirm hardware register value if desired.
	FPGA registers are 16-bits wide and all 16-bits must be written atomically.  To write individual bits in a register use the axWriteFPGAregBIT() function.
	axWriteFPGAreg() is functionally equivalent to the AxsunOCTControl.dll library function "SetFPGARegister()" used when connected to the DAQ via Ethernet or USB.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axWriteFPGAreg(AOChandle session, uint16_t regnum, uint16_t regval);

	/**
	\brief Write a single bit in an FPGA register on the Axsun DAQ via the PCIe interface. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param regnum The unique register number to which the write operation is directed.
	\param bitnum The bit number within the desired register.  The 16 bits in the register are indexed from 0 (LSB) to 15 (MSB).
	\param bitval The value to write (0 = clear bit, 1 = set bit)
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that this function does not subsequently query the DAQ hardware to confirm the write was actually successful. A return value of NO_AxERROR only indicates the software's request was made successfully. Call axReadFPGAreg() to confirm hardware register value if desired.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axWriteFPGAregBIT(AOChandle session, uint16_t regnum, uint8_t bitnum, uint8_t bitval);

	/**
	\brief Configures FPGA registers to output the desired data type & location from the processing pipeline via the PCIe interface. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param mode The desired pipeline mode according to the numbered pipeline diagram shown as in the Operator's Manual.
	\param channels The desired channel selection mode (see AxChannelMode enum)
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that this function does not subsequently query the DAQ hardware to confirm the write was actually successful. A return value of NO_AxERROR only indicates the software's request was made successfully. Call axReadFPGAreg() to confirm hardware register value if desired.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axPipelineMode(AOChandle session, AxPipelineMode mode, AxChannelMode channels);

	/**
	\brief Read a FPGA register on the Axsun DAQ via the PCIe interface. (WINDOWS ONLY)
	\param session The opaque handle to the session created with axStartSession().
	\param regnum The unique register number to which the read operation is directed.
	\param regval Will be populated with the register value fetched from the FPGA.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details axReadFPGAreg is functionally equivalent to the AxsunOCTControl.dll library function "GetFPGARegister" used when connected to the DAQ via Ethernet or USB.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axReadFPGAreg(AOChandle session, uint16_t regnum, uint16_t *regval);

	/**
	\brief Export images from the Main Image Buffer into individual binary or graphics files.
	\param session The opaque handle to the session created with axStartSession().
	\param path_directory Nul-terminated path to directory in which to export files (must be less than 200 characters).
	\param images_exported Will be populated with the number of images successfully exported.
	\param file_type The desired file type (.bin, .jpg, .bmp, .png)
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that any preexisting files in the directory indicated for exporting could be overwritten without warning.
	Do not call this function concurrently with new data being captured into the buffer (i.e. when the DAQ is in imaging mode) as buffer contents may change in an undesired fashion during export.
	Descriptors such as image dimensions (height & width) or data type are not exported in the raw .bin file; the user must associate these descriptors manually if needed. 2 or 4-byte data types are exported in big-endian format.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axExportImages(AOChandle session, const char* path_directory, int32_t* images_exported, AxFileType file_type);

	/**
	\brief Export images from the Main Image Buffer into individual graphics and movie files, with dimensional (cropping and scaling), intensity (brightness and contrast), and colormap adjustments.
	\param session The opaque handle to the session created with axStartSession().
	\param path_directory Nul-terminated path to directory in which to export binary files (must be less than 200 characters).
	\param images_exported Will be populated with the number of images successfully exported.
	\param file_type The desired file type. Raw Binary files can be exported for any data type. PNG files can be exported for U8 and U16 data types. JPEG and BMP files can be exported only for U8 data type.
	\param start_image The unique image number of the image at which to start exporting. If = 0, all images will be exported.
	\param end_image The unique image number of the image at which to stop exporting. If = 0, all images will be exported.
	\param request_prefs Preferences related to image averaging, cropping, and other behaviors (same scheme used in axRequestImage()).
	\param export_prefs Controls properties such as size, colormap, contrast, and brightness of the exported images.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details Note that any preexisting files in the directory indicated for exporting could be overwritten without warning.
	Do not call this function concurrently with new data being captured into the buffer (i.e. when the DAQ is in imaging mode) as buffer contents may change in an undesired fashion during export.
	Descriptors such as image dimensions (height & width) or data type are not exported in the raw .bin file; the user must associate these descriptors manually if needed. 2 or 4-byte data types are exported in big-endian format.
	*/
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axExportImagesAdv(AOChandle session, const char* path_directory, int32_t* images_exported, AxFileType file_type, uint32_t start_image, uint32_t end_image, request_prefs_t request_prefs, export_prefs_t export_prefs);


	/**
	\brief Control analog output waveform generation for 2 channel (X-Y) scanners. (WINDOWS ONLY)
	\param scan_command The desired scanner function selected from available commands in the AxScannerCommand enum.  See below for notes on available scanner commands.
	\param misc_scalar A general purpose scalar value.  Valid when scan_command = WAIT_BURST, SET_SAMPLE_CLOCK, GET_DEVICE_SERIALNO, or SELECT_DEVICE. See below for usage.
	\param scan_parameters A structure defining the basic geometry of a rectangular raster scan pattern generated by the library (parameter valid when scan_command = SET_RECT_PATTERN or STOP_AT_POSITION, otherwise set to NULL).
	\param external_scan_pattern A structure defining the user-generated scan pattern and associated arrays to be loaded (parameter valid when scan_command = LOAD_EXT_PATTERN, otherwise set to NULL).
	\param RFU Reserved for future use, set to NULL.
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.  Some scan commands return positive integers as noted in their description.
	\details Scan patterns are configured based on voltage values at the output pins of the analog output generation device. Converting a voltage value to an optical beam position must incorporate external linear and non-linear factors such as the type of scanner & amplifier/controller, the opto-mechanical layout, lens distortion, etc.
	The active scan pattern (an array of interleaved X and Y voltages) can be generated by the library based on user-configurable high-level geometric parameters (i.e. range and origin) to define a rectangular raster scan area, or generated externally by the user and then loaded directly via array format.
	The rectangular raster scan pattern generated by the library consists of linear ramp functions (sawtooth wave with 100% duty cycle) along the X and Y scan dimensions.  When scanning, the "fast" X voltage is updated for each clock pulse and the "slow" Y voltage is updated after each full period of the X waveform.

	SETUP and HARDWARE CONNECTIONS:
	- supports NI USB-6211, USB-6001, USB-6008, or USB-6343 hardware (including OEM style) according to the following matrix.
	- requires installation of NI-DAQmx device driver software. (http://sine.ni.com/nips/cds/view/p/lang/en/nid/10181)
	- make the following terminal connections according to the functionality desired and the board type used:

		FUNCTION							USB-6211		USB-6343		USB-6001/6008		GROUND TYPE
			X scan waveform	output				AO 0			AO 0			<n/a>			A GND
			Y scan waveform	output				AO 1			AO 1			<n/a>			A GND
			Auxiliary analog DC output			<n/a>			AO 2			AO 0			A GND
			Image_sync pulse output				PFI 5			PFI 13			<n/a>			D GND
			Ext. sample clock input				PFI 0			PFI 8			<n/a>			D GND
			Square wave output					PFI 4			PFI 12			<n/a>			D GND
			Auxiliary digital output			PFI 6			PFI 2			P 1.2			D GND

	AVAILABLE SCANNER COMMANDS:
	- axScanCmd(<STRONG>INIT_SCAN</STRONG>, ...)  Initialize hardware and allocate scanner control resources. (DEPRECATED)
	
		Scanner control resources are lazy-initialized automatically on the first call to any axScanCmd() command type, so this INIT_SCAN command is no longer needed.
		Hardware resources are initialized automatically when a supported device is detected (may take up to 10 seconds for device to be recognized after connecting).
		Use SET_MAX_VOLTAGE command once a device is connected in order to set its maximum voltage limits for analog output waveforms.

	- axScanCmd(<STRONG>DESTROY_SCAN</STRONG>, ...) Destroy and deallocate scanner control resources.
	
		Resources previously allocated with other axScanCmd(...) function calls are automatically deallocated by axStopSession() when terminating a main Axsun OCT Capture session.  Call this command to explicitly deallocate scanner resources if a main capture session was not used. 

	- axScanCmd(<STRONG>SET_RECT_PATTERN</STRONG>, ..., scan_parameters, ...) Use scan_parameters to configure a rectangular raster scan pattern.

		The basic geometry defined in the scan_parameters structure is used to generate the active 1D (line) and 2D (raster) scan patterns for subsequent analog output generation, overwriting active scan patterns previously generated with this command or loaded with axScanCmd(LOAD_EXT_PATTERN,...).  Contents of the scan_parameters structure are copied internally and the pointer to this structure need not remain valid following return from this command.

	- axScanCmd(<STRONG>LOAD_EXT_PATTERN</STRONG>, ..., external_scan_pattern, ...) Use external_scan_pattern to load an externally generated scan pattern.

		An arbitrary user-generated scan pattern can be loaded for subsequent analog output generation, overwriting active scan patterns previously loaded with this command or generated with axScanCmd(SET_RECT_PATTERN,...).  Contents of the external_scan_pattern structure and associated arrays are copied internally and the pointer to this structure and its associated arrays need not remain valid following return from this command.  

	- axScanCmd(<STRONG>CONTINUOUS_LINE_SCAN</STRONG>, ...) Start continuous line scanning based on configured scan pattern.

		Starts the analog output generation for the active 1D linear scan, repeating it continuously until commanded otherwise. The Image_sync pulse frequency is derived from the X_increment configured when the active scan pattern was set or loaded.

	- axScanCmd(<STRONG>CONTINUOUS_RASTER_SCAN</STRONG>, ...) Start continuous raster scanning based on configured scan pattern. 

		Starts the analog output generation for the active 2D raster scan, repeating it continuously until commanded otherwise. The Image_sync pulse frequency is derived from the X_increment configured when the active scan pattern was set or loaded..

	- axScanCmd(<STRONG>STOP_AT_POSITION</STRONG>, ..., scan_parameters, ...) Move to a configured position and stop scanning.

		Stops the Image_sync pulse and sets a constant (non-scanning) analog output at the voltages given in the X_shift and Y_shift fields of the scan_parameters argument. This command does not alter or overwrite the active scan parameters or patterns previously configured using axScanCmd(SET_RECT_PATTERN,...).  

	- axScanCmd(<STRONG>SETUP_BURST_RASTER</STRONG>, ...) Prepare a burst raster scan by pre-loading buffers and waiting at the start position.
	
		Uploads the active raster scan pattern to device memory and sets a constant analog output at the pattern's initial voltage, but waits to start scanning until a subsequent call to axScanCmd(START_BURST_RASTER,...).

	- axScanCmd(<STRONG>START_BURST_RASTER</STRONG>, ...) Start a burst raster scan with minimum latency.

		Starts analog output and Image_sync pulse generation for a burst raster scan.  Must be preceded by a call to axScanCmd(SETUP_BURST_RASTER,...). The raster scan starts with minimal latency and is executed once.

	- axScanCmd(<STRONG>WAIT_BURST</STRONG>, misc_scalar, ...) Wait for a burst raster scan to complete.

		Waits for a burst raster scan operation to compete. The misc_scalar parameter sets the timeout (in seconds) to wait before returning an error. A timeout of -1 waits indefinitely, a timeout of 0 returns immediately with an error if the raster scan is still active or no error if it is complete.

	- axScanCmd(<STRONG>SET_SAMPLE_CLOCK</STRONG>, misc_scalar, ...) Changes between external and internal sample clock.

		The misc_scalar parameter sets the desired sample clock for subsequent analog output generation. The default value of 0 uses the external sample clock (connected to pin PFI0), a non-zero value uses a 100 kHz sample clock generated internally by the device. Active scanning operations must be restarted for changes to take effect.

	- axScanCmd(<STRONG>GET_DEVICE_SERIALNO</STRONG>, misc_scalar, ...) Returns a device serial number (if AxErr return value > 0, type-cast it to `int`).

		A list of N connected devices is maintained internally by the library.  The misc_scalar parameter sets the device index (from 0 to N-1) on which to query the device serial number. If the index is higher than N-1, function will return AxErr::DAQMX_DEVICE_NOT_FOUND.

	- axScanCmd(<STRONG>SELECT_DEVICE</STRONG>, misc_scalar, ...) Selects the active device.

		A list of N connected devices is maintained internally by the library.  The device occupying the first position (index 0) is considered the "active device" to which other axScanCmd() commands are directed.
		The misc_scalar parameter sets the new "active device" (indexed from 0 to N-1) to be moved into the first position in the list.  
		If the index is higher than N-1, function will return AxErr::DAQMX_DEVICE_NOT_FOUND.
		If the selected device is not supported, function will return AxErr::DAQMX_UNSUPPORTED_DEVICE.

	- axScanCmd(<STRONG>COUNT_DEVICES</STRONG>, ...) Returns the number of connected DAQmx devices (if AxErr return value > 0, type-cast it to `int`).

		Counts the number of DAQmx devices currently connected, including both supported and unsupported devices.

	- axScanCmd(<STRONG>SET_MAX_VOLTAGE</STRONG>, ...) Sets the maximum analog waveform voltage a device is allowed to output.

		A device's maximum analog waveform output is set to +/- 10.0 V by default.  Use the misc_scalar parameter to set a new maximum value from 0.0 V to 10.0 V on the active device.  The minimum voltage will be set to -misc_scalar.
		This feature is a safeguard against unintentional over-driving of connected scanners; subsequent commands to start scanning will return an error if the programmed scan pattern exceeds the configured voltage limit.  This command does not apply to the general purpose DC analog output functionality provided by some devices.

	- axScanCmd(<STRONG>SET_DIGITAL_OUTPUT</STRONG>, misc_scalar, ...) Controls the general purpose digital output.

		Sets the device's general purpose digital output to low/false if misc_scalar = 0 or high/true if misc_scalar = 1.

	- axScanCmd(<STRONG>SET_SQUAREWAVE_OUTPUT</STRONG>, misc_scalar, ...) Controls the general purpose square wave synthesizer.

		Sets the device's general purpose square wave synthesizer to output at a frequency given by misc_scalar in Hz.  Use misc_scalar = 0 to turn the squarewave output off.

	- axScanCmd(<STRONG>SET_XSINE_YRAMP_PATTERN</STRONG>,  ..., scan_parameters, ...) Configures a raster scan pattern with sinusoidal waveform on the X dimension and linear ramp waveform on the Y dimension.

		Usage is similar to axScanCmd(SET_RECT_PATTERN, ...).

	- axScanCmd(<STRONG>SET_AUX_DC_ANALOG_OUTPUT</STRONG>, misc_scalar, ...) Controls the general purpose DC analog output voltage.

		Sets the device's general purpose DC analog output to a voltage given by misc_scalar in Volts.

	- axScanCmd(<STRONG>SET_AUX_DC_ANALOG_MAX</STRONG>, misc_scalar, ...) Controls the general purpose DC analog output voltage maximum limit.

		Sets the device's maximum general purpose DC analog output to a voltage given by misc_scalar in Volts. This feature is a safeguard against unintentional over-driving of this output terminal; subsequent commands to SET_AUX_DC_ANALOG_OUTPUT will return an error if the requested voltage exceeds the configured limit.

	- axScanCmd(<STRONG>SET_AUX_DC_ANALOG_MIN</STRONG>, misc_scalar, ...) Controls the general purpose DC analog output voltage minimum limit.

		Sets the device's minimum general purpose DC analog output to a voltage given by misc_scalar in Volts. This feature is a safeguard against unintentional over-driving of this output terminal; subsequent commands to SET_AUX_DC_ANALOG_OUTPUT will return an error if the requested voltage exceeds the configured limit.

	*/
		
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axScanCmd(AxScannerCommand scan_command, double misc_scalar, scan_params_t * scan_parameters, ext_pattern_t * external_scan_pattern, void * RFU);

	/**
	 \brief Registers a callback function to be executed following a scanner (DAQmx) device connection or disconnection event. (WINDOWS ONLY)
	 \param callback_function A user-supplied function to be called.  Pass NULL to un-register a callback function.
	 \param userData An arbitrary structure to be made available inside the callback function (or NULL if unused).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	 \details The supplied callback function will be executed by a background thread when a new DAQmx device is detected or when an existing device is disconnected.
	 Within the callback function, access is provided to arbitrary user data passed as a void* parameter to this function.
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axRegisterScannerConnectCallback(AxConnectScannerCallbackFunction_t callback_function, void * userData);

	/**
	 \brief Registers a callback function to be executed upon a change in the DAQ imaging state (e.g. transition to live, record, idle).
	\param session The opaque handle to the session created with axStartSession().
	 \param callback_function A user-supplied function to be called.  Pass NULL to un-register a callback function.
	 \param user_data An arbitrary structure to be made available inside the callback function (or NULL if unused).
	 \return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	 \details The supplied callback function will be executed when the DAQ image state changes, or when axReportImagingState() is called.  The int32_t argument available in the registered AxCallbackFunction_t will convey the DAQ imaging state per the AxDAQEvent enum.
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		AxErr axRegisterDAQCallback(AOChandle session, AxCallbackFunction_t callback_function, void * user_data);

	/**
	 \brief Programmatically executes the pre-registered callback function to indicate the current DAQ imaging state.
	\param session The opaque handle to the session created with axStartSession().
	 \return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	 \details Before calling this function, register your custom callback function using axRegisterDAQCallback().  This function aids event-driven program flow in the client application, as an alternative to polling the 'imaging' parameter in axGetStatus().
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axReportImagingState(AOChandle session);

	/**
	 \brief Registers a callback function to be executed each time an image is exported using axExportImages() or axExportImagesAdv().
	\param session The opaque handle to the session created with axStartSession().
	 \param callback_function A user-supplied function to be called.  Pass NULL to un-register a callback function.
	 \param user_data An arbitrary structure to be made available inside the callback function (or NULL if unused).
	 \return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	 \details The supplied callback function will be executed when an image is exported. This provides client software with the ability to monitor the export status, update a progress bar, etc.  The callback function is executed in the main exporting thread and thus delays in the user callback will delay return of the associated axExportImages() or axExportImagesAdv() call.  The int32_t argument available in the registered AxCallbackFunction_t will convey the image number of the exported image.
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axRegisterExportCallback(AOChandle session, AxCallbackFunction_t callback_function, void * user_data);

	/**
	\brief Registers a callback function to be executed each time a new image is enqueued in the Main Image Buffer, whether captured from DAQ hardware or loaded from disk.
	\param session The opaque handle to the session created with axStartSession().
	\param callback_function A user-supplied function to be called.  Pass NULL to un-register a callback function.
	\param user_data An arbitrary structure to be made available inside the callback function (or NULL if unused).
	\return AxErr = NO_AxERROR for success or other AxErr error code on failure.  Call axGetErrorString() for a description of an AxErr error code.
	\details This function enables event-driven program flow in the client application, as an alternative to polling the Main Image Buffer for new images using axGetImageInfo(). The data argument available in the registered AxNewImageCallbackFunction_t will convey the unique image number of the newly enqueued image.
 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */
		AxErr axRegisterNewImageCallback(AOChandle session, AxNewImageCallbackFunction_t callback_function, void * user_data);

	/**
	 \brief Get the build configuration of the library.
	 \return = a bitfield with information about the build configuration options used to compile the loaded library.
	 \ Bit 0: = 0 for 'debug' configuration or = 1 for 'release' configuration
	 \ Bit 1: (for internal use only)
	 \ Bit 2: = 0 for non-verbose mode or = 1 for verbose logging mode
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		int32_t axGetLibBuildCfg(void);

	/**
	 \brief Get a string describing the time and date that the library was built.
	 \param datetime_str A pointer to a pre-allocated output buffer of characters with size at least 100 bytes into which a nul-terminated date & time string will be copied.
	 \return The number of characters (bytes) written into the datetime_str buffer.
	 \details It is unsafe to pass a datetime_str output buffer allocated with fewer than 100 bytes.
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		int32_t axGetLibBuildDateTime(char* datetime_str);

	/**
	 \brief Get the library version.
	 \param major Version number major element.
	 \param minor Version number minor element.
	 \param patch Version number patch element.
	 \param build Version number build element.
	 \param version_str A pointer to a pre-allocated output buffer of characters with size at least 16 bytes into which a nul-terminated version string will be copied.
	 \details It is unsafe to pass a version_str output buffer allocated with fewer than 16 bytes.
	 */
	/*! \cond */ AXSUN_EXPORTS /*! \endcond */ 
		void axGetLibVersion(int32_t * major, int32_t * minor, int32_t * patch, int32_t * build, char* version_str);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif  // AXSUNOCTCAPTURE_H include guard