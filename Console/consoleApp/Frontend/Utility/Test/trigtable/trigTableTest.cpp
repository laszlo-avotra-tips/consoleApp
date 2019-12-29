/*
 * trigTableTest.cpp
 *
 * Unit test for trig lookup table object.
 */

#include <math.h>
#include "..\trigLookupTable.h"
#include "trigtabletest.h"

#define PI 3.141592654

const float tolerance( 0.1 );

void trigTableTest::lookupSin()
{
  trigLookupTable &myTable = trigLookupTable::Instance();
  float mySinValue( 0.0 );
  float theirSinValue( 0.0 );
  float angleStep( 0.01 );
  float angle;

  for ( int step = 0; step < ( 1080 / angleStep ); step++ ) {
    
    angle = step * angleStep;
    
    // Convert angle to radians
    angle = angle * ( PI/180.0 );

    mySinValue = sin( angle );
    theirSinValue = myTable.lookupSin( angle );

    QString valString = QString( "Table Value: %1 Check Value: %2 for angle %3, step %4" ).arg( theirSinValue ).arg( mySinValue ).arg( angle ).arg ( step );
    QVERIFY2( fabs( mySinValue - theirSinValue ) <= tolerance, valString.toAscii() );
  }
}

void trigTableTest::lookupCos()
{
  trigLookupTable &myTable = trigLookupTable::Instance();
  float myCosValue( 0.0 );
  float theirCosValue( 0.0 );
  float angleStep( 0.01 );
  float angle;

  for ( int step = 0; step < ( 1080 / angleStep ); step++ ) {
    
    angle = step * angleStep;
    
    // Convert angle to radians
    angle = angle * ( PI/180.0 );

    myCosValue = cos( angle );
    theirCosValue = myTable.lookupCos( angle );

    QString valString = QString( "Table Value: %1 Check Value: %2 for angle %3, step %4" ).arg( theirCosValue ).arg( myCosValue ).arg( angle ).arg ( step );
    QVERIFY2( fabs( myCosValue - theirCosValue ) <= tolerance, valString.toAscii() );
  }
}

void trigTableTest::lookupPos()
{
  trigLookupTable &myTable = trigLookupTable::Instance();
  QPoint myPoint;
  QPoint theirPoint;
  int linePos(0);

  float angleStep( 0.01 );
  float angle;

  for ( int step = 0; step < ( 1080 / angleStep ); step++ ) {
    
      for ( linePos = 0; linePos < 512; linePos++ ) {
        angle = step * angleStep;
        
        // Convert angle to radians
        angle = angle * ( degToRad );

        theirPoint = myTable.lookupPosition( linePos, angle );

        myPoint.setX( floor( ( cos( angle ) * (float)linePos ) + 0.5 ) );
        myPoint.setY( floor( ( sin( angle ) * (float)linePos ) + 0.5 ) );
        QPoint diffPoint = myPoint - theirPoint;

        // Allows ~1 degree of error
        QVERIFY( diffPoint.manhattanLength() < 6);
      }
  }
}

QTEST_MAIN(trigTableTest)

