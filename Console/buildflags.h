/*
 * buildflags.h
 *
 * Flags to change the overall behavior of the OCT HS System Software
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */
#ifndef BUILDFLAGS_H
#define BUILDFLAGS_H


/*
 * Common flags to all the applications
 */
#define USE_MOUSE_CAPTURE 0
#define ENABLE_EXE_CHECKS 0

/*
 * HomeScreen Flags
 */

/*
 * Enable/Disable the password check in HomeScreen
 */
#define ENABLE_PASSWORD_CHECK 0

/*
 * consoleApp Flags
 */

/*
 * Turn on/off init checks. Init is very picky about file locations
 * and file signing.
 */
#if _DEBUG
#define USE_INIT 0
#else
#define USE_INIT 1
#endif

/*
 * Enable System Key Checks
 */
#if _DEBUG
#   define ENABLE_SYSTEM_KEY_CHECK 0
#else
#   define ENABLE_SYSTEM_KEY_CHECK 1
#endif


#define ENABLE_SINGLE_MONITOR_WARNING 0

/*
 * R&D flags
 */

/*
 * Debug output of frame processing speeds
 */
#define ENABLE_LOGGING_TO_DEBUG_WINDOW       0
#define ENABLE_COLORMAP_OPTIONS              0
#define ENABLE_VIDEO_CRF_QUALITY_TESTING     0
#define ENABLE_SINGLE_STREAM_RECORDING       0
#define ENABLE_ON_SCREEN_RULER               0
#define ENABLE_SLED_SUPPORT_BOARD_TESTING    0
#define ENABLE_MEASUREMENT_PRECISION         0

/*
 * Force background recording off (ignore value of fullCaseRecording in system.ini)
 */
#define DISABLE_BACKGROUND_RECORDING         1

/*
 * Enable/disable Squish wrapper for starting octConsole.exe as a sub-process.
 * When enabled, the squish wrapper is used which allows Squish to hook into the
 * sub-process.
 * See locations.h for the location of the Squish wrapper.
 * See homewindow.cpp for the consoleApp.start details.
 */
#define ENABLE_SQUISH 0

/*
 * Demo mode: release build for Jason Burk. Runs without any Lightbox hardware.
 */
#define ENABLE_DEMO_MODE 0

#define ENABLE_LAPTOP_MODE 0

#endif // BUILDFLAGS_H
