/*
 * trigLookupTable.cpp
 *
 * Implements a simple accelerator for converting polar coordinates to Cartesian
 * using a lookup table. The number of points in the table is goverened by the
 * desired angular resolution and line length (in pixels).
 *
 * The lookup table implements the singleton design pattern so it will only be
 * built once per application.
 *
 * Author: Chris White
 *
 * Copyright 2009-2018 Avinger Inc.
 */


#include <math.h>
#include "trigLookupTable.h"
#include "util.h"

const int overhead( 3 ); // How far beyond line length to extend the table.

/*
 * Constructor
 */
trigLookupTable::trigLookupTable ( float minResolution, int lineLength )
{
    int count;

    resolution = minResolution;
    lineLen = lineLength * overhead; // Extra entries to handle image morph

    // Depends on angular resolution
    numEntries = (int)floor( (2*pi/resolution) + 0.5 ) + 1;

    sinTable = new float[ numEntries ];
    cosTable = new float[ numEntries ];

    for (count = 0; count < numEntries; count++) {
        sinTable[count] = sin( (float)count * resolution );
        cosTable[count] = cos( (float)count * resolution );
    }

    buildPositionTable();
}

/*
 * Destructor
 */
trigLookupTable::~trigLookupTable ( void )
{
    int i;

    delete [] sinTable;
    delete [] cosTable;
    for ( i = 0; i < lineLen; i++ ) {
        delete [] posTable[i];
    }
    delete [] posTable;
}

/*
 * buildPositionTable
 *
 * Generates the table used to convert from rho, theta
 * coordinate system to screen position. Uses the sin and
 * cos tables built earlier.
 */
void trigLookupTable::buildPositionTable( void )
{
    int i, j;
    int x, y;

    posTable = new QPoint *[ lineLen ];

    for ( i = 0; i < lineLen; i++ )
    {
        posTable[i] = new QPoint[ numEntries ];
        for( j = 0; j < numEntries; j++ )
        {
            x = floor_int( (float)i * cos(j * resolution) + 0.5 );
            y = floor_int( (float)i * sin(j * resolution) + 0.5 );
            posTable[i][j].setX( x );
            posTable[i][j].setY( y );
        }
    }
    return;
}

/*
 * lookupPosition
 *
 * Given a polar coordinate, return a QPoint containing
 * the corresponding screen location.
 */
QPoint trigLookupTable::lookupPosition( int rho, float radians )
{
    // Only operate modulo 360 degrees
    while( radians > 2*pi )
    {
        radians -= 2 * pi;
    }
    while( radians < 0 )
    {
        radians += 2*pi;
    }

    // Position is a simple lookup into the position table
    int iTheta = floor_int( ( radians / resolution ) + 0.5 ); 
    return( posTable[rho][iTheta] );
}

/*
 * lookupCos
 *
 * Given an angle in radians return the closest cos value
 * in the table.
 */
float trigLookupTable::lookupCos ( float radians )
{
    int index;

    // Only operate modulo 360 degrees
    while ( radians > 2*pi)
        radians -= 2*pi;
    while ( radians < 0 )
        radians += 2*pi;

    index = floor_int( ( radians/resolution ) + 0.5 );
    return( cosTable[index] );
}

/*
 * lookupSin
 *
 * Given an angle in radians return the closest sin value
 * in the table.
 */
float trigLookupTable::lookupSin ( float radians )
{
    int index;

    // Only operate modulo 360 degrees
    while ( radians > 2*pi )
        radians -= 2*pi;
    while ( radians < 0 )
        radians += 2*pi;

    index = floor_int( ( radians/resolution ) + 0.5 );
    Q_ASSERT(index < numEntries);
    return( sinTable[index] );
}

/*
 * lookupSinCos
 * 
 * Convenience function to lookup both the sin and cos
 * of an angle in radians. Both are returned in the passed
 * by refernces sinOut and cosOut.
 */
void trigLookupTable::lookupSinCos ( float radians, float & sinOut, float &cosOut )
{
    int index;

    // Only operate modulo 360 degrees
    while ( radians > 2*pi )
        radians -= 2*pi;
    while ( radians < 0 )
        radians += 2*pi;

    index = floor_int( ( radians/resolution ) + 0.5 );
    sinOut = sinTable[index];
    cosOut = cosTable[index];
}
