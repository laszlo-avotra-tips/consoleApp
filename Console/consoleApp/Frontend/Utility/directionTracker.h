/*
 * directionTracker.h
 *
 * Monitors and reports the direction of travel of the encoder.
 *
 * Author(s): Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 *
 */

#ifndef DIRECTIONTRACKER_H_
#define DIRECTIONTRACKER_H_

class directionTracker
{
public:
    enum Direction_T {
        CounterClockwise = -1,
        Stopped = 0,
        Clockwise = 1
    };

    directionTracker();
    void getEncoderCountsForDevice( void );
    void updateDirection( unsigned short newPosition );
    Direction_T getDirection( void ) { return currDirection; }

private:
    Direction_T currDirection;
    short prevPosition;
    unsigned short encoderCounts;
    unsigned short crossingThreshold_counts;
};

#endif // DIRECTIONTRACKER_H_
