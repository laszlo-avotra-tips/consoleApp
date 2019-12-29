/*
 * lowspeeddaq.h
 *
 * Implements the DAQ interface for low speed devices. Low speed devices
 * acquire and process single A-lines at a time and provide the A-lines
 * fully baked to the frontend.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#ifndef LOWSPEEDDAQ_H
#define LOWSPEEDDAQ_H

#include "daq.h"

/*
 * LowSpeedDAQ
 */
class LowSpeedDAQ : public DAQ
{
public:
    LowSpeedDAQ();
    ~LowSpeedDAQ();
    void init( void );
    bool configure( void );
    bool getData( void );

private:
    unsigned short encoderCounts;
    U16 *m_daqRawData[2];

    // prevent access to copy and assign
    LowSpeedDAQ( LowSpeedDAQ const & );
    LowSpeedDAQ & operator=( LowSpeedDAQ const & );
};

#endif // LOWSPEEDDAQ_H
