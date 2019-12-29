/*
 * powerdistributionboard.h
 *
 * Header for the Power Distribution Board class. All that is done here is
 * power on and power off the Power Distribution Board. This should be done
 * in main() at the start and end of the case.
 *
 * Author: Ryan F. Radjabi
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 *
 */
#ifndef POWERDISTRIBUTIONBOARD_H
#define POWERDISTRIBUTIONBOARD_H

class powerDistributionBoard
{
public:
    void powerOn( void );
    void powerOff( void );
};

#endif // POWERDISTRIBUTIONBOARD_H
