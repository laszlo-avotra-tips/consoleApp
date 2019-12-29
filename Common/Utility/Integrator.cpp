/*
 * Integrator.cpp
 *
 * The integrator is a simple counting object that
 * does not update to a full units worth of change
 * until it crosses over a limit. I.e. we expect
 * changes to accumulate a full unit before reacting.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#include "Integrator.h"
#include <math.h>
#include "util.h"

const double DefaultLimit( 1.0 );

Integrator::Integrator( void )
{
    currValue = 0.0;
    limit = DefaultLimit;
}

Integrator::Integrator( double newLimit )
{
    currValue = 0.0;
    limit = newLimit;
}

Integrator::~Integrator(void)
{
}

/*
 * integrate
 *
 * Given a new value, add it to the accumulator, if
 * it is over the limit, go ahead and give it the
 * full value.
 */
void Integrator::integrate( double newValue )
{
    currValue += newValue;

    if( fabs( currValue ) > limit )
    {
        currValue = sign( currValue ) * limit;
    }
}
