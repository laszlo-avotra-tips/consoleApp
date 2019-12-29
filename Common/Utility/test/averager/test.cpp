/*
 * test.cpp
 *
 * Unit test for the averager utility class.
 */

#include <QtTest/QtTest>
#include "averager.h"

#define N_ELEMENTS(pArr) ((sizeof((pArr))/(sizeof((pArr)[0]))))

const int testWidth(10); // Test samples wide filter

class TestAverager : public QObject
{

  Q_OBJECT

  private slots:
  void testSimple();
  void testWithAngles();

};

void TestAverager::testSimple (void)
{
  averager uut;
  const int testIterations( testWidth * 2 );

  // Test some range of values
  double testValues[6] = {0.0, 5.0, 13.0, 17.0, 25.0, 50.0};

  uut.disableAngleCorrection(); // Simple averager
  uut.setWidth( testWidth );

  QCOMPARE( uut.getNextValue( 0.0 ), 0.0 );

  uut.setWidth( testWidth ); // reset the object.

  // Verify that continuous values produce the same result
  for ( int val = 0; val < N_ELEMENTS( testValues ); val++) {

    double value = testValues[val];
    for ( int i = 0; i < testIterations; i++ ) {
      if ( i < testWidth ) {
	QCOMPARE( uut.getNextValue( value ), ( value * (double)( i + 1 ) ) / testWidth );
      } else {
	QCOMPARE( uut.getNextValue( value ), value );
      }
    }
    
    // Reset between runs
    uut.setWidth( testWidth );
  }
}

void TestAverager::testWithAngles (void)
{
  averager uut;
  const int testIterations( testWidth * 2 );
  double testValues[6] = {0.0, 5.0, 13.0, 17.0, 25.0, 50.0}; // Test some range

  uut.setWidth( testWidth );

  QCOMPARE( uut.getNextValue( 0.0 ), 0.0 );

  uut.setWidth( testWidth ); // reset the object.

  // Verify that continuous values produce the same result
  for ( int val = 0; val < N_ELEMENTS( testValues ); val++) {

    double value = testValues[val];
    for ( int i = 0; i < testIterations; i++ ) {
      if ( i < testWidth ) {
	QCOMPARE( uut.getNextValue( value ), ( value * (double)( i + 1 ) ) / testWidth );
      } else {
	QCOMPARE( uut.getNextValue( value ), value );
      }
    }
    
    // Reset between runs
    uut.setWidth( testWidth );
  }

  // Now verify that wrap-around values are handled correctly
  for ( int i = 0; i < testIterations; i++ ) {
     if ( i < testWidth ) {
       QCOMPARE( uut.getNextValue( 5.0 ), ( 5.0 * (double)( i + 1 ) ) / testWidth );
      } else {
	QCOMPARE( uut.getNextValue( 5.0 ), 5.0 );
      }
  }
  
  // Verify that a wrap around value ( > 270 degrees instantaneous difference )
  // resets the entire average buffer to that value.
  QCOMPARE( uut.getNextValue(300.0), 300.0);
}

QTEST_MAIN(TestAverager)
#include "test.moc"
