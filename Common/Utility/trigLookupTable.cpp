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
    numEntries = int(floor( (2 * pi / double(resolution)) + 0.5 ) ) + 1;

    sinTable = new float[ size_t(numEntries) ];
    cosTable = new float[ size_t(numEntries) ];

    for (count = 0; count < numEntries; count++) {
        sinTable[count] = sin( count * resolution );
        cosTable[count] = cos( count * resolution );
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

    posTable = new QPoint *[ size_t(lineLen) ];

    for ( i = 0; i < lineLen; i++ )
    {
        posTable[i] = new QPoint[ size_t(numEntries) ];
        for( j = 0; j < numEntries; j++ )
        {
            x = floor_int( i * cos(j * resolution) + 0.5f );
            y = floor_int( i * sin(j * resolution) + 0.5f );
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
QPoint trigLookupTable::lookupPosition( int rho, double radians )
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
    int iTheta = floor_int( ( float(radians) / resolution ) + 0.5f );
    return( posTable[rho][iTheta] );
}

/*
 * lookupCos
 *
 * Given an angle in radians return the closest cos value
 * in the table.
 */
float trigLookupTable::lookupCos ( double radians )
{
    int index;

    // Only operate modulo 360 degrees
    while ( radians > 2*pi)
        radians -= 2*pi;
    while ( radians < 0 )
        radians += 2*pi;

    index = floor_int( ( float(radians) / resolution ) + 0.5f );
    return( cosTable[index] );
}

/*
 * lookupSin
 *
 * Given an angle in radians return the closest sin value
 * in the table.
 */
float trigLookupTable::lookupSin ( double radians )
{
    int index;

    // Only operate modulo 360 degrees
    while ( radians > 2*pi )
        radians -= 2*pi;
    while ( radians < 0 )
        radians += 2*pi;

    index = floor_int( ( float(radians) / resolution ) + 0.5f );
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
void trigLookupTable::lookupSinCos ( double radians, float & sinOut, float &cosOut )
{
    int index;

    // Only operate modulo 360 degrees
    while ( radians > 2*pi )
        radians -= 2*pi;
    while ( radians < 0 )
        radians += 2*pi;

    index = floor_int( ( float(radians) / resolution ) + 0.5f );
    sinOut = sinTable[index];
    cosOut = cosTable[index];
}
