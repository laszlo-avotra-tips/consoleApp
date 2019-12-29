/*
 * unwindMachine.h
 *
 * The unwind machine is a state machine designed to keep track of and compensate
 * for a configured amount of rotation windup in a rotating device. The machine 
 * monitors rotation (via the update() call) and when a direction change is detected
 * it accumulates angles up to the configured "lag" value. Once that value is reached
 * it provides the caller with an offset to make up for it. The machine also provides
 * the caller with status, indicating whether or not lag is currently being taken up.
 * For readability and maintainability reasons, all angles are in degrees throughout
 * this module.
 *
 * Author: Chris White and Dennis Jackson
 * Copyright (c) 2009-2017 Avinger Inc.
 */

#pragma once

class unwindMachine
{

public:
    unwindMachine( void );
    ~unwindMachine( void );

    void reset( void );
    bool update( double newPosition );
    bool isUnwinding( void );
    void enableLargeUnwind( bool enable );
    void setWindAngle( double angle );
    void setEnabled( bool val ) {
        enabled = val;

        // Reset the state.
        state = Idle;
        unwoundAngle = 0.0;
    }

    double getWindAngle( void );
    double getOffset( void );
    double getRemainingUnwind( void );
    double getStartingAngle( void );

    // Return the direction that the devices was rotating at the
    // start of the unwind operation.
    int getInitialDirection( void ) {
        return ( initialDirection );
    }

    // Return the amount of angle current unwound
    double getUnwoundAngle( void ) { 
        return ( unwoundAngle );
    }

private:

    enum states {
        Idle = 0,
        WaitingForDirectionChange,
        Unwinding
    };

    bool updateDirection( double newPosition );
    bool doUnwind( double newPosition );
    int direction( double newPosition );

    bool enabled;
    enum states state;
    double currentAngle;
    double largeUnwind;
    double windAngle;
    double unwoundAngle;
    double postUnwindFixup;
    int currDirection;
    int initialDirection;
};
