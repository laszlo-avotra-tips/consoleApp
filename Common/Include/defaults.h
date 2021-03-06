#ifndef DEFAULTS_H
#define DEFAULTS_H
#include <QString>
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QPainter>

#define _WINDOWS
#define SECTOR_HEIGHT_PX 1024
#define FFT_DATA_SIZE    1024
#define MAX_LINES_PER_FRAME 12000   // normalized to 1000rpm that is 6000 lines/frame
                                    // memory is allocated for twice the lines/frame

#define SECTOR_SIZE_B (SECTOR_HEIGHT_PX * SECTOR_HEIGHT_PX)

const int SectorWidth_px  = SECTOR_HEIGHT_PX;
const int SectorHeight_px = SECTOR_HEIGHT_PX;

#define MAX_ACQ_IMAGE_SIZE ( FFT_DATA_SIZE * MAX_LINES_PER_FRAME) // max acquired frame size

#define SURFACE_BOOK 0
#define SIMULATION_MODE 0
#define LINE_AVERAGING 0
#define RECORDING_ON 1
#define USE_SLED_SUPPORT_BOARD 1
#define USE_NEW_SLED_SUPPORT_BOARD 1
#define LASER_SCAN_DIVIDER 0    // Set scan rate to 100/(divider+1 ) kHz

const int ControlScreenWidth = 3240;
const int ControlScreenHeight = 2160;
const double IMAGE_SCALE_FACTOR{2.2};


// file defines
const int B_per_KB = 1024;
const int KB_per_MB = 1024;

// The size, in sizeof(OCTFile_t) units, of the shared memory buffer for the data producer
// and consumer.
#define FRAME_BUFFER_SIZE 16

#define MININTERPOLATIONANGLE_RAD (0.0005)

// Window Flags macro required for virtual keyboard input context
const Qt::WindowFlags KeyboardWindowFlags = Qt::Widget | Qt::Dialog | Qt::FramelessWindowHint;

// TDB: Constants to be moved to device settings ( Refs #1062 )
const int ALineLength_px  = 512;

// Keep a maximum length that is independent of devices
const int MaxALineLength = 1024;

const int MinDriveSpace_GB = 10;
const int MinNumberMonitors = 2;

const int TechScreenWidth  = 3240;
const int TechScreenHeight = 2160;

const int HideMouseDelay_ms = 3000;

// Constants for conversions
#define MS_PER_S (1000)
#define S_PER_MIN (60)

// Constants for system and data locations
#ifdef WIN32
const QString ExpectedApplicationDirPath = "C:/Program Files/Avinger/OCTConsole";
const QString DataDir              = "C:/Avinger_Data";
const QString SystemDir            = "C:/Avinger_System";
const QString TrashDir             = "C:/Avinger_Trash";
const QString TrainingDir          = "C:/Avinger_Training";
const QString TrainingCaseStudiesDir = TrainingDir + "/case_studies";
const QString TrainingOctReviewDir   = TrainingDir + "/oct_review";
const QString TrainingReferenceDir   = TrainingDir + "/reference";

#else
const QString ExpectedApplicationDirPath = ".";
const QString DataDir              = "/opt/Avinger_Data";
const QString SystemDir            = "/opt/Avinger_System";
const QString TrainingDir          = "/opt/Avinger_Training";
#endif
const QString DevicesPath          = SystemDir + "/devices";
const QString ExportCacheDir       = "exportcache";
const QString ExportCachePath      = DataDir + "/" + ExportCacheDir;
const QString ExportArchivePath    = "Avinger_Exports";
const QString BackupDirectory      = "Avinger_Backup/";
const QString KeyFile              = "keys.txt";
const QString L300KeyFile          = "L300Keys.txt";
const QString SystemKeysPath       = SystemDir + "/" + KeyFile;
const QString EventDataLogFileName = "events.xml";
const QString SessionDatabaseFileName = "session.db";
const QString SystemSettingsFile   = SystemDir + "/system.ini";
const QString SystemVarFile   = SystemDir + "/systemVar.ini";
const QString SystemProfileFile   = SystemDir + "/systemProfile.ini";
const QString ApprovedAppsIniFile  = SystemDir + "/approvedApps.ini";
const QString KioskCookieFile      = SystemDir + "/kioskversion.txt";
const QString TrainingCookieFile   = TrainingDir + "/training_version.txt";
const QString SystemLogFileName    = SystemDir + "/OCT_System.log";
const QString OpticalDrivePath     = "D:/";
const QString UsbDrivePath         = "E:/";
const QString OctHsInstallerFile   = "OCT-HS-Console-setup.exe";
const QString MasterDatabase       = DataDir + "/master.db";
const QString CaseCookieFilename   = DataDir + "/casecookie.txt";

const QString CaseVideoExtension       = ".mkv";
const QString LoopVideoExtension       = ".mp4";
const QString LoopVideoExtension_v2    = ".mkv";  // Loop file extension for v2.x ~ v3.x software for backwards compatibility
const QString ExportLoopVideoExtension = ".mp4";

const QString DeviceDescriptionExtension = ".xml";
const QString DeviceIconExtension        = ".png";
const QString DeviceXmlSchemaVersion     = "3.0";

// Image capture
const QString ImagePrefix          = "IMG";
const int ThumbnailHeight_px = 190;
const int ThumbnailWidth_px  = 190;

// Customer service
const QString ServiceNumber = "650-241-7900";
const QString InvestigationalDeviceWarning = QObject::tr( "CAUTION: Investigational Device. Limited by United States Law to investigational use only. Exclusively for Clinical Investigations." );

// Display constants
typedef struct
{
    int minValue;
    int maxValue;
    int defaultValue;
} DisplayLevel_T;

const DisplayLevel_T BrightnessLevels_HighSpeed = { 0, 255, 10 };
const DisplayLevel_T BrightnessLevels_LowSpeed  = { 0, 65535, 5000 };

const DisplayLevel_T ContrastLevels_HighSpeed = { 0, 255, 250 };
const DisplayLevel_T ContrastLevels_LowSpeed  = { 0, 65535, 55000 };
const DisplayLevel_T ImageIndexDecimationLog  = { 0, 17, 65535 };


const int  DefaultReticleBrightness        = 127;
const int  DefaultLaserIndicatorBrightness = 255;
const int  DefaultCurrFrameWeight_Percent  = 75;
const bool DefaultUseNoiseReduction        = false;
const bool DefaultUseInvertOctColor        = false;

// DAQ and DSP constants that are needed outside of the DAQ and DSP
const unsigned int FFTDataSize = 1024;
const unsigned int FFTDataSize_bytes = FFTDataSize * sizeof( short );

// Size of the sector on the 27" (2560x1440) monitor
//const int SectorHeight_px = 1408;  // must be factor of 16 (GPU requirement)
//const int SectorWidth_px  = 1408;

/*
 * Video settings
 */
const double DefaultLsVideoFPS = 15.0;
const double DefaultHsVideoFPS = 20.0;
const int HsVideoTimeoutCount = 5;

// Minimum size for a video file before deciding it is corrupt (this is
// about the size of a single frame mkv file).
const unsigned int MinVideoFileSizeBytes = 45000;


/*
 * Settings in octConsole.ini
 */
const QString ConsoleIniFile = SystemDir + "/octConsole.ini";
const QString LightboxSerialNumberKey( "Console/Lightbox Serial Number" );
const QString DefaultLaserSerialNumber = "N/A";
const QString LaserSerialNumberKey( "Console/Laser Serial Number" );
const QString DefaultLightboxSerialNumber = "0000";
const QString DaqServiceDateKey( "DAQ/Last Service Date" );
const QString DefaultDaqServiceDate( "2008-12-03" );

/*
 * OCT Measurements constants
 */
const int SupportedMeasurementVersion = 1;

/*
 * EVOA Settings
 */
const QString EvoaDefaultSetting( "EVOA/DefaultVoltage" );
const QString EvoaStatusDefault = "Default";
const QString EvoaStatusSet     = "Set";
const QString EvoaStatusOff     = "Off";

/*
 * Laser settings
 */
const QString DefaultPortName        = "COM1";
const QString DefaultLaserCommConfig = "9600,8,n,1";

/*
 * OCT Loop recording constants
 */
const int MinRecordingLength_s  = 1;
const int MinRecordingLength_ms = MinRecordingLength_s * 1000;

/*
 * Strings
 */
const QString CatheterPointedUpText   = "Catheter Up";
const QString CatheterPointedDownText = "Catheter Down";

/*
 * Active and Passive colors
 */
const QColor AggressiveSpinColor = QColor( 201, 156, 72 ); // yellow-ish
const QColor PassiveSpinColor    = QColor( 45, 135, 134 );  // light blue-ish
#define ActiveColor  8E8E4E        // yellowish
#define PassiveColor 2A8C91        // bluish
#endif // DEFAULTS_H
