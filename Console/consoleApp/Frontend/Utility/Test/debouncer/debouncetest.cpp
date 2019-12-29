/*
 * debounceTest.cpp
 *
 * Unit test for the debounce object.
 */

#include "..\debouncer.h"
#include "debouncetest.h"

const int testIntLimit( 5 );
const double testDoubleLimit( 5.0 );

void debounceTest::testIntDebounce()
{
  debouncer uut;
  int iterations( 128 );
  int signVal( -1 );

  uut.setLimit( testIntLimit );

  /* 
   * Bounce back and forth within the limit and
   * make sure that the debouncer, uh, debounces.
   */
  for ( int i = 0; i < iterations; i++ ) {
    QVERIFY(!uut.debounce( ( testIntLimit / 2 ) * signVal ) );
    signVal *= -1;
  }

  /* 
   * Exceed the limit from below, expect debounce to return TRUE
   */
  uut.setLimit( testIntLimit );

  for ( int i = 0; i <= testIntLimit; i++ ) {
    uut.debounce( i );
  }

  QVERIFY( uut.debounce( testIntLimit + 1 ) );

  /*
   * Exceed the limit from above, expect debounce to return TRUE
   */
  uut.setLimit( testIntLimit );
  
  for ( int i = -testIntLimit; i <= 0; i++ ) {
    uut.debounce( i );
  }

  QVERIFY( uut.debounce( testIntLimit + 1 ) );

  /*
   * Verify the reset() keeps the debouncer from overflowing
   */
  for ( int i = 0; i < iterations; i++ ) {
    uut.debounce( i );
    uut.reset();
    QVERIFY( !uut.debounce( 1 ) );
  }
}

void debounceTest::testAngleDebounce()
{
  debouncer uut;
  int iterations( 128 );
  double signVal( -1.0 );

  uut.setLimit( testDoubleLimit );

  /* 
   * Bounce back and forth within the limit and
   * make sure that the debouncer, uh, debounces.
   */
  for ( int i = 0; i < iterations; i++ ) {
    QVERIFY(!uut.debounce( ( ( testDoubleLimit / 2.0 ) - 1.0 ) * signVal ) );
    signVal *= -1.0;
  }

  /* 
   * Exceed the limit from below, expect debounce to return TRUE
   */
  uut.setLimit( testDoubleLimit );

  for ( int i = 0; i <= testDoubleLimit; i++ ) {
    uut.debounce( i );
  }

  QVERIFY( uut.debounce( testDoubleLimit + 1.0 ) );

  /*
   * Exceed the limit from above, expect debounce to return TRUE
   */
  uut.setLimit( testDoubleLimit );
  
  for ( int i = -testDoubleLimit; i <= 0; i++ ) {
    uut.debounce( i );
  }

  QVERIFY( uut.debounce( testDoubleLimit + 1 ) );

  /*
   * Verify the reset() keeps the debouncer from overflowing
   */
  for ( int i = 0; i < iterations; i++ ) {
    uut.debounce( i );
    uut.reset();
    QVERIFY( !uut.debounce( 1.0 ) );
  }
}

QTEST_MAIN(debounceTest)
