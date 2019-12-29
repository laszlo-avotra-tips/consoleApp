#include <QtTest/QtTest>
#include "Integrator.h"

class TestIntegrator: public QObject
{
    Q_OBJECT

private slots:
    void reset();
    void value();
    void setLimit();
//    void init() { qDebug("in init"); }
};

void TestIntegrator::reset()
{
    // check default constructor
    Integrator i;

    // put something in that is bigger than the default
    i.integrate( 100.0 );
    QCOMPARE( i.value(), 1.0 );

    // make sure it resets
    i.reset();
    QCOMPARE( i.value(), 0.0 );
}


void TestIntegrator::value()
{
    // instantiate with a value and make sure it is zeroed out
    Integrator i( 10.0 );
    QCOMPARE( i.value(), 0.0 );

    // add some values in
    i.integrate( 1.1 );
    i.integrate( 2.2 );
    QCOMPARE( i.value(), 3.3 );

    // check the limit is used
    i.integrate( 9.9 );
    QCOMPARE( i.value(), 10.0 );

    i.integrate( -5.0 );
    QCOMPARE( i.value(), 5.0 );

    // go check the negative of the limit
    i.integrate( -15.0 );
    QCOMPARE( i.value(), -10.0 );
}

void TestIntegrator::setLimit()
{
    // instantiate and make sure it is zeroed out
    Integrator i( 2.5 );
    QCOMPARE( i.value(), 0.0 );

    // check the original limit
    i.integrate( 5.0 );
    QCOMPARE( i.value(), 2.5 );

    // change the limit and make sure the value stays the same
    //   TBD: what should happen if the new limit is less than the current value?
    i.setLimit( 7.0 );
    QCOMPARE( i.value(), 2.5 );

    // check the new limit
    i.integrate( 5.0 );
    QCOMPARE( i.value(), 7.0 );

    // check setting a negative limit
    i.setLimit( -5.0 );
    i.integrate( -50.0 );
    QCOMPARE( i.value(), -5.0 );

    // and back to the positive with a negative limit set
    i.integrate( 50.0 );
    QCOMPARE( i.value(), 5.0 );


}



QTEST_MAIN(TestIntegrator)
#include "test.moc"
