/*
 * locations.h
 *
 * Canonical file paths and install locations for utilities, applications, etc.
 *
 * Copyright 2011 Avinger Inc.
 */
#ifndef LOCATIONS_H
#define LOCATIONS_H

/*
 * Canonical path for console app
 */
const QString consoleAppLocation("C:/Program Files/Avinger/OCTConsole/octConsole.exe");

/*
 * Where to find the Squish wrapper for starting the consoleApp (to allow hooks)
 */
#if ENABLE_SQUISH
const QString SquishWrapperLocation( "C:/Squish-OCTHS/bin/dllpreload.exe" );
#endif

#endif // LOCATIONS_H
