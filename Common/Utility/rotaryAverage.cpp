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
 * Copyright (c) 2011-2018 Avinger, Inc.
 */
#include "rotaryAverage.h"
#include <memory.h>
#include <math.h>
#include "profiler.h"
#include <QDebug>

// Note: Turning on debugging causes a lot of blurring. It's useful for short intervals
//       with a hand-turned encoder.
#define DEBUG_AVERAGE 0

rotaryAverage::rotaryAverage( void )
{
    values = nullptr;
    width = DefaultWidth;
    range = DefaultRange;
    maxValue = range - 1;
    idx = 0;
    sum = 0;
    averageValue = 0;
}

rotaryAverage::~rotaryAverage( void )
{
    if (values)
    {
        delete [] values;
    }
}
/*
 * reset
 *
 * Configure the rotaryAverage with a new averaging width and/or range of encoder values.
 *      newWidth        averaging width, the number of latest values to keep up with
 *      newRange        how many encoder counts make one full revolution, value range 0..(newRange-1)
 */
void rotaryAverage::reset( int newWidth, int newRange )
{
    if (values)
    {
        delete [] values;
        values = nullptr;
    }

    if ( newWidth <= 0 )
    {
        newWidth = DefaultWidth;
    }
    values = new double[ size_t(newWidth) ];
    memset(values, 0, sizeof( double ) * size_t(newWidth));
    width = newWidth;

    range = newRange;
    maxValue = range - 1;

    idx = 0;
    sum = 0;
    averageValue = 0;

    #if DEBUG_AVERAGE
    qDebug() << "\n########### rotaryAverage::reset width:" << width << "maxValue:" << maxValue << "range:" << range
             << "\n########### width:" << width << "idx:" << idx << "sum:" << sum << "averageValue:" << averageValue;
    for( int i = 0; i < width; i++ )
    {
        qDebug() << "###########     values[" << i << "]:" << values[ i ];
    }
    #endif
}

/*
 * getNextValue
 *
 *
 */
double rotaryAverage::getNextValue( double newValue )
{
    if ( !values )
    {
        return( 0.0 );
    }
    if( width <= 1 )
    {
        return( newValue );
    }

    #if DEBUG_AVERAGE
    qDebug() << "\n::::::::::: rotaryAverage::getNextValue newValue:" << newValue
             << "\n::::::::::: width:" << width << "idx:" << idx << "sum:" << sum << "averageValue:" << averageValue;
    for( int i = 0; i < width; i++ )
    {
        qDebug() << ":::::::::::     values[" << i << "]:" << values[ i ];
    }
    #endif

    // See if this incoming new value indicates we just wrapped.
    // averageValue holds the running average value, the last value returned by this method.

    const int WrapThreshold = int(0.75f * range);
    double deltaValue = newValue - averageValue;

    #if DEBUG_AVERAGE
    qDebug() << "::::::::::: deltaValue:" << deltaValue;
    #endif

    // slideValue indicates if we wrapped, and if so which way is back to the neighborhood
    int slideValue = 0;
    if ( fabs( deltaValue ) < WrapThreshold )
    {
        // no wrap, no need to do anything
    }
    else if( deltaValue < 0 )
    {
        slideValue = 1; // fell off the high end, slide up to get back to the neighborhood
    }
    else
    {
        slideValue = -1; // fell off the low end, slide down to get back to the neighborhood
    }

    #if DEBUG_AVERAGE
    qDebug() << "::::::::::: slideValue:" << slideValue;
    #endif

    // In order to average the values arithmetically, we create a virtual value from the incoming new value.
    // If we wrapped, then we'll translate the virtual value to be 'close' to the values already stored
    // by sliding one revolution (range) up or down. The virtual value does not need to stay
    // in the encoder range, it's only used for averaging.

    int virtualNewValue = int(newValue);
    if( slideValue != 0 )
    {
        virtualNewValue += ( slideValue * range );

        #if DEBUG_AVERAGE
        qDebug() << "########### virtualNewValue:" << virtualNewValue;
        #endif
    }

    // Now just do the arithmetic housekeeping
    sum           -= int(values[ idx ]);
    values[ idx ]  = virtualNewValue;
    sum           += int(values[ idx ]);
    idx            = ( idx + 1 ) % width; // Note: trying to use 'idx++' for 'idx + 1' will cause chaos and crashing
    averageValue   = int( floor( ( float(sum) / float(width) ) + 0.5f ) );

    #if DEBUG_AVERAGE
    qDebug() << "::::::::::: width:" << width << "idx:" << idx << "sum:" << sum << "averageValue:" << averageValue;
    for( int i = 0; i < width; i++ )
    {
        qDebug() << ":::::::::::     values[" << i << "]:" << values[ i ];
    }
    #endif

    // Since this average is computed from virtual encoder values that may be out of range, the average
    // may have gone out of range. This means the whole neighborhood has moved to the other end.
    // slideAverage will tell us which way to go to get the average back in range.

    int slideAverage = 0;
    if( averageValue > maxValue )
    {
        slideAverage = -1; // average is too high, slide everything down to get back in range
    }
    else if( averageValue < 0 )
    {
        slideAverage = 1; // average is too low, slide up to get happy
    }

    #if DEBUG_AVERAGE
    qDebug() << "::::::::::: slideAverage:" << slideAverage;
    #endif

    // Adjust everything if necessary.
    if( slideAverage != 0 )
    {
        int adjust = slideAverage * range;

        #if DEBUG_AVERAGE
        qDebug() << "########### adjust:" << adjust;
        #endif
        sum += adjust * width;
        for( int i = 0; i < width; i++ )
        {
            values[ i ] += adjust;
        }
        averageValue += adjust;

        #if DEBUG_AVERAGE
        qDebug() << "########### width:" << width << "idx:" << idx << "sum:" << sum << "averageValue:" << averageValue;
        for( int i = 0; i < width; i++ )
        {
            qDebug() << "###########     values[" << i << "]:" << values[ i ];
        }
        #endif
    }

    return averageValue;
}
