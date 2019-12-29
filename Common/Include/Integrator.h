/*
 * Integrator.h
 *
 * The integrator is a simple counting object that
 * does not update to a full units worth of change
 * until it crosses over a limit. I.e. we expect
 * changes to accumulate up to a full unit before reacting.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2017 Avinger, Inc.
 */
#pragma once

#include <math.h>

class Integrator
{
public:
    Integrator( void );
    Integrator( double newLimit );
    ~Integrator(void);
    void setLimit( double newLimit ) { limit = fabs( newLimit ); }
    void integrate( double newValue );
    void reset( void ) { currValue = 0; }
    double value( void ) { return currValue; }

private:
    double limit;
    double currValue;
};
