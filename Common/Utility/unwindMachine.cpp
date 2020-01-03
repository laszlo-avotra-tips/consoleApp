/*
 * unwindMachine.cpp
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
 * Copyright 2000-2018 Avinger Inc.
 */

#include "unwindMachine.h"
#include <math.h>
#include "util.h"
#include <QDebug>

/*
 * Constants, #defines
 */
const double ChangeTolerance( 0.001 );
const double LargeUnwindValue( 180.0 );

/*
 * Constructor
 */
unwindMachine::unwindMachine ( void )
{
    reset();
}

/* 
 * Destructor
 */
unwindMachine::~unwindMachine ( void )
{
}

/*
 * reset()
 *
 * Resets the machine. Clears all accumulated values and
 * restores the state to Idle.
 */
void unwindMachine::reset ( void )
{
    state           = Idle;
    currentAngle    = 0.0;
    windAngle       = 0.0;
    unwoundAngle    = 0.0;
    postUnwindFixup = 0.0;
    currDirection   = 1;
    largeUnwind     = 0.0;
    enabled         = true;
}

/*
 * getRemainingUnwind()
 *
 * Returns the amount of lag left to be taken up by rotation in degrees
 */
double unwindMachine::getRemainingUnwind ( void )
{
    return( ( windAngle + largeUnwind ) - unwoundAngle );
}

/*
 * getStartingAngle()
 *
 * Returns the angle at which a lag take-up operation started. Primarily
 * used for onscreen indicators and such.
 */
double unwindMachine::getStartingAngle ( void )
{
    return( currentAngle );
}

/*
 * direction()
 *
 * Given a new position (from a client), updates the internal notion
 * of direction of rotation. Handles the case of rotating past the
 * origin.
 */
int unwindMachine::direction ( double newPosition )
{
    // Determine instantaneous direction (from the previous from frame to this one)
    // This is only used by unwindMachine.
    double angleDiff = newPosition - currentAngle;
    int newDirection;

    // Special case: crossing over 360 (heuristically defined as 
    // instantaneous angle change > 270 degrees
    // If we crossed over then complement the difference with one
    // revolution (360 degrees) and flip the sign.
    if ( fabs( angleDiff ) > 270.0 ) {
        angleDiff = -sign( angleDiff ) * ( 360.0 - fabs( angleDiff ) );
    }
    
    if ( angleDiff > 0 ) {
        newDirection = 1;
    } else {
        newDirection = -1;
    }

    return( newDirection );
}

/*
 * updateDirection()
 *
 * Given a new position, checks to see if the direction has changed. If it has,
 * return true. In either case, update the internal direction state accordingly.
 */
bool unwindMachine::updateDirection ( double newPosition )
{
    bool changed = false;
    
    if ( currDirection != direction( newPosition ) ) {
        changed = true;
        currDirection = -currDirection;
    }
 
    return( changed );
}

/*
 * doUnwind()
 *
 * The core of the logic. Given a new position from the client, determine the direction
 * of motion, the change in position from last time, and accumulated rotation.
 * Given the results of the calculation, determine whether unwinding is complete or
 * cancelled and update the state accordingly.
 */
bool unwindMachine::doUnwind ( double newPosition )
{
    double angleDiff = newPosition - currentAngle;

    // Special case: crossing over 360 (heuristically defined as 
    // instantaneous angle change > 270 degrees
    // If we crossed over then complement the difference with one
    // revolution (360 degrees) and flip the sign.
    if ( fabs( angleDiff ) > 270.0 ) {
        angleDiff = -sign( angleDiff ) * ( 360.0 - fabs( angleDiff ) );
    }

    // If the direction is different from when we
    // started unwrapping, we need to remove that motion
    // from the accumulator. Do this using the direction
    // as the sign of the motion.
    unwoundAngle += direction( newPosition ) * currDirection * fabs( angleDiff );

    // If we've gone all the way back around, cancel the unwind.
    if ( unwoundAngle < 0.0 ) {
        state = WaitingForDirectionChange;
        unwoundAngle = 0.0;
        currDirection *= -1;
        //qDebug() << "=====> unwind canceled - currDirection:" << currDirection << "\n";
        return( false );
    }

    // If we're past the configured lag angle, then the operation 
    // is complete.
    if ( unwoundAngle >= ( windAngle + largeUnwind ) ) {
        state = WaitingForDirectionChange;

        // The below is not ideal, we should track total position w/o
        // wrapping and use that to determine when to start/end lag
        // correction.
        postUnwindFixup += unwoundAngle * currDirection;
        unwoundAngle = 0.0;
        //qDebug() << "=====> unwind completed - postUnwindFixup: " << postUnwindFixup << "currDirection:" << currDirection << "\n";
        return( true );
    } else {

        // Not done yet? Just update the position.
        currentAngle = newPosition;
        return( false);
    }
}

/*
 * update()
 *
 * State machine for the unwinder. This call is made by the client with the new angle
 * of rotation. Depending on the current state and the new position vs. the old position,
 * the state machine leaves or enters the unwind state.
 * 
 * Most angle inputs are debounced once unwinding starts, to eliminate onscreen jitter.
 */
bool unwindMachine::update ( double newPosition )
{
    bool applyOffset = false;

    // If we've not actually moved, just update 
    // the internal state variables and move on.
    if ( ( fabs( windAngle + largeUnwind ) < ChangeTolerance ) || !enabled ) {
        updateDirection( newPosition );
        currentAngle = newPosition;
        return( false );
    }

    switch ( state ) {
        case Idle:
            currentAngle = newPosition;
            state = WaitingForDirectionChange;
            break;
        
        case WaitingForDirectionChange:
            if ( updateDirection( newPosition ) ) {
                state = Unwinding;
                initialDirection = currDirection;
                //qDebug() << "\n=====> Start unwinding - newPosition:" << newPosition << "initialDirection:" << initialDirection;
            }
            currentAngle = newPosition;
            break;

        case Unwinding:
            applyOffset = doUnwind( newPosition );
            break;
    }

    // Update the client with the offset to keep angles
    // consistent after unwind.
    return( applyOffset );
}

/*
 * isUnwinding()
 *
 * Let the client know if unwinding is occurring.
 */
bool unwindMachine::isUnwinding ( void )
{
    return ( state == Unwinding );
}

/*
 * setWindAngle()
 *
 * Accessor to allow the client to set the take-up angle.
 */
void unwindMachine::setWindAngle( double angle ) 
{
    reset();
    windAngle = angle;
}

/*
 * enableLargeUnwind
 *
 * Add 180 degrees to the current angle
 */
void unwindMachine::enableLargeUnwind( bool enable )
{
    // save current angle through the call to reset
    double oldAngle = windAngle;
    reset();
    if ( enable ) {
        largeUnwind = LargeUnwindValue;
    } else {
        largeUnwind = 0.0;
    }
    windAngle = oldAngle;
}

/*
 * getWindAngle()
 *
 * Accessor to allow the client to query the currently configured
 * take-up angle.
 */
double unwindMachine::getWindAngle ( void ) 
{
    return( windAngle + largeUnwind );
}

/*
 * getOffset()
 *
 * Accessor to return the currently accumulated correction offset.
 * Without this, after unwind occurs, the client would draw at
 * the wrong angle. This allows the client to take into account
 * all previous unwind operations.
 */
double unwindMachine::getOffset ( void )
{
    return( postUnwindFixup );
}
