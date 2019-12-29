/*
 * rotaryAverage.cpp
 *
 * Implements a simple rotary averaging filter of configurable width and range.
 * Transitions over the origin in both directions (0-to-maxValue and maxValue-to-0) are
 * handled smoothly with true arithmetic averaging.
 *
 * This implementation is largely just an embellishment of averager.cpp
 *
 * Author: Jim Smith
 *
 * Copyright (c) 2011 Avinger, Inc.
 */
#pragma once

const int DefaultWidth = 4;
const int DefaultRange = 1024;

class rotaryAverage
{
public:
    rotaryAverage( void );
    ~rotaryAverage( void );

    void reset( int newWidth = DefaultWidth, int newMaxValue = DefaultRange );
    double getNextValue( double newValue );

private:
    int width;              // averaging width, the number of latest values to keep up with
    int range;              // how many encoder counts make one full revolution, value range 0..(range-1)

    double *values;         // circular stash of latest values, sized to hold 'width' number of them
    int idx;                // circular pointer to the next slot to use in 'values'
    int sum;                // sum of latest values
    double averageValue;    // mean of latest values
    int maxValue;           // max for any value, (range-1)
};
