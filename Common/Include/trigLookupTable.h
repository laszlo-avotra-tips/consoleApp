/*
 * trigLookupTable.h
 *
 * Defines a simple accelerator for converting polar coordinates to Cartesian
 * using a lookup table. The number of points in the table is goverened by the
 * desired angular resolution and line length (in pixels).
 *
 * The lookup table implements the singleton design pattern so it will only be
 * built once per application.
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2017 Avinger, Inc.
 */
#pragma once

#include <QPoint>
#include "defaults.h"
#include "util.h"

class trigLookupTable
{
public:
  
    // Singleton
    static trigLookupTable & Instance() {
        static trigLookupTable theTable;
        return theTable;
    }
    float lookupCos( float radians );
    float lookupSin( float radians );
    void lookupSinCos( float radians, float & sinOut, float & cosOut );
    QPoint lookupPosition( int rho, float radians );

private:

    // Hide ctor, dtor, copy and assign for singletons
    trigLookupTable( float resolution = MININTERPOLATIONANGLE_RAD, int lineLength = ALineLengthNormal_px );
    ~trigLookupTable( void );
    trigLookupTable( trigLookupTable const & );
    trigLookupTable & operator=( trigLookupTable const & );
    void buildPositionTable( void );

    float *sinTable;
    float *cosTable;
    QPoint **posTable;
    float resolution;
    int lineLen;
    int numEntries;
};
