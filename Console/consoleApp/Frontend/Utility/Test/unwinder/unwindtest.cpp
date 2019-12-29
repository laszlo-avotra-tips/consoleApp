/*
 * unwindTest.cpp
 *
 * Unit test for the lag correction object.
 *
 * Author: Chris White
 *
 * Copyright 2009 Sawtooth Labs, Inc.
 */

#include "..\unwindMachine.h"
#include "unwindTest.h"

void unwindTest::testUnwind()
{
  double unwindAngle( 0 );
  unwindMachine uut; 
  
  /*
   * Set up a series simulated unwind sessions from 10 degrees
   * through 180. First test: rotate back and forth within the unwind,
   * verify correct behavior. Second: rotate further in one direction,
   * verify the exit of unwind state. Third: rotate in one direction, 
   * then back in the other fully past the unwind to verify cancellation
   * behavior.
   */

  for ( int i = 1; i <= 18; i++ ) {
    unwindAngle = i * 10.0;

    /*
     * Verify setup
     */
    uut.reset();
    
    QVERIFY( !uut.isUnwinding() );
    QCOMPARE( uut.getRemainingUnwind(), 0.0 );
    QCOMPARE( uut.getWindAngle(), 0.0 );

    uut.setWindAngle( unwindAngle );
    QCOMPARE( uut.getWindAngle(), unwindAngle );

    /*
     * Cycle within the unwind zone, expect to be told we are
     * in unwind mode.
     */
    double angle = 0.0;

    // Go in one direction to prime.
    for ( int j = 0 ; j <= 10; j++ ) {
      angle = ( unwindAngle / 10.0 ) * j;
      
      QVERIFY( !uut.update( angle ) );
      QVERIFY( !uut.isUnwinding() );
    }

    // Reverse to start unwind
    angle = angle - 1.0;
    uut.update( angle );
    
    QVERIFY( uut.isUnwinding() );

    double signVal = 1.0;

    /*
     * Repeat test twenty time to make sure accumlated state
     * does not cause problems.
     */
    for ( int cycleCount = 0; cycleCount < 20; cycleCount++ ) {
     
      signVal *= -1.0;
      
      // Rotate back and forth within unwindAngle degrees
      for ( int j = 0; j < 9; j++ ) {
        angle = angle + signVal * ( ( unwindAngle ) / 10.0 );

	/*
	 * Should *not* apply offset during unwind and should report
	 * unwinding.
	 */
	QVERIFY( !uut.update( angle ) );
	QVERIFY( uut.isUnwinding() );
      }
    }

    /*
     * Finally watch as we rotate in one direction to verify that 
     * we state in unwind and come out at the right moment.
     */
    uut.reset();
    uut.setWindAngle( unwindAngle );
    QCOMPARE( uut.getWindAngle(), unwindAngle );
    
    // Go in one direction to prime.
    for ( int j = 0 ; j <= 10; j++ ) {
      angle = ( unwindAngle / 10.0 ) * j;
      
      QVERIFY( !uut.update( angle ) );
      QVERIFY( !uut.isUnwinding() );
    }

    // Reverse to start unwind
    angle = angle - 1.0;
    uut.update( angle );
    
    double startAngle = uut.getStartingAngle();

    QVERIFY( uut.isUnwinding() );
    bool once = false;

    for ( int j = 0; j <= 15; j++ ) {
        angle = angle - signVal * ( ( unwindAngle ) / 10.0 );

	/*
	 * Should *not* apply offset during unwind and should report
	 * unwinding.
	 */
	if ( fabs( startAngle - angle ) < unwindAngle ) {
	  QVERIFY( !uut.update( angle ) );
	  QVERIFY( uut.isUnwinding() );
	} else {  

        /*
         * Should report apply offset only the first
         * call after switching out of unwind.
         */
        if (!once) {
            QVERIFY( uut.update( angle ) );
            once = true;
        } else {
            QVERIFY( !uut.update( angle ) );
        }
	    QVERIFY( !uut.isUnwinding() ); 
      }    
    }
  }
}

QTEST_MAIN(unwindTest)
