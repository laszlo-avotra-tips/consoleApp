/*
 * directionTracker.cpp
 *
 * Monitors and reports the direction of travel of the encoder.
 *
 * Author(s): Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */

#include <QMetaType>
#include <QDebug>
#include "util.h"
#include "directionTracker.h"
#include "daqSettings.h"
#include "deviceSettings.h"

// Local constants
static const short MinAngleChange_cnt  = 10;
static const int   MaxIgnoreCount      = 50;
static const int   DefaultEncoderCount = 1024;

/*
 * Constructor
 */
directionTracker::directionTracker()
{
    // register the direction data type for use in signals and slots
    qRegisterMetaType<directionTracker::Direction_T>( "directionTracker::Direction_T" );

    currDirection = Stopped;
    prevPosition  = 0;
    encoderCounts = 0;
}

/*
 * getEncoderCountsForDevice
 *
 * Making the assignment to encoderCounts must be done after a device is selected.
 *
 * getEncoderCountsForDevice() is called by updateDirection() if encoderCounts == 0(uninitialized).
 */
void directionTracker::getEncoderCountsForDevice( void )
{
    deviceSettings &devSettings = deviceSettings::Instance();
    if( devSettings.current() )
    {
//        encoderCounts = ushort(devSettings.current()->getLinesPerRevolution());
        encoderCounts = DefaultEncoderCount;
    }
    else
    {
        encoderCounts = DefaultEncoderCount;
    }
    crossingThreshold_counts = ushort(0.75 * encoderCounts);
}

/*
 * updateDirection
 *
 * Method to determine which direction the encoder is rotating.
 */
void directionTracker::updateDirection( unsigned short newPosition )
{
    // Make sure the assignment has been made.
    if( encoderCounts == 0 )
    {
        getEncoderCountsForDevice();
    }

    // The number of lines streaming through here is quite high; checking the
    // angle change between each one is not valid since the rate of rotation
    // is much, much lower than the sampling rate.  Since this is the case,
    // there are many points in time that the angle difference is 0 counts
    // even though the encoder is rotating.  So we only check for changes
    // every now and then and ignore the rest.
    static int ignoreCount = 0;
    ignoreCount++;
    if( ignoreCount < MaxIgnoreCount )
    {
        return;
    }
    // else process this set of data to determine the direction of rotation

    // reset the counter
    ignoreCount = 0;

    // Determine the change from the previous measurement
    short angleDiff_cnt = short(newPosition - prevPosition);

    // Special case: crossing over 360 (heuristically defined as
    // instantaneous angle change > 270 degrees
    // If we crossed over then complement the difference with one
    // revolution (encoderCounts counts) and flip the sign.
    if( abs( angleDiff_cnt ) > crossingThreshold_counts )
    {
        angleDiff_cnt = short(-sign( angleDiff_cnt ) * ( encoderCounts - abs( angleDiff_cnt ) ) );
    }

    // maintain state across calls
    static Direction_T newDirection = Stopped;

    // Check for no change or change in direction
    if( abs(angleDiff_cnt) < MinAngleChange_cnt ) {
        newDirection = Stopped;
    }
    else if( angleDiff_cnt > MinAngleChange_cnt ) {
        newDirection = Clockwise;
    }
    else if( angleDiff_cnt < -MinAngleChange_cnt ) {
        newDirection = CounterClockwise;
    }
    // else maintain last state

    prevPosition  = newPosition;
    currDirection = newDirection;
}
