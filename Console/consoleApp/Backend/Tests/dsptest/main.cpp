#include <QtTest/QtTest>
#include <stdio.h>
#include <math.h>
#define UNIT_TEST
#include "dsp.h"
#include "daqSettings.h"

/*
   2011-09-12 djackson: 
   This test must be compiled from Visual Studio. The VS project  
   can be created using the command:
      qmake -tp vc dsptest.pro
   VS uses the dspSettings.cpp stub file; qtcreator and nmake from the command
   line use the real dspSettings.cpp when moc runs. Why? Good question.
*/


const QString logTableName("logtable.txt");
const QString transformTableName("transformtable.txt");

const int logTableSize(65535);
const int freqTableSize(2048);
const int transformLength(512);

class TestDSP : public QObject
{
    Q_OBJECT

public:
    TestDSP() {
    }
    ~TestDSP() {
    }

private slots:

void testEncoder();
void testLogTable();
void testTransform();
void testRescale();

private:

};

void TestDSP::testLogTable(void)
{
    DSP uut;
    double dblLogArray[logTableSize];
    QFile logFile(logTableName);

    uut.init();

    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QFAIL("Logarithm table file missing");
        return;
    }

    QTextStream s(&logFile);

    // Load the comparison result array
    QString logStr = s.readLine();
    QStringList logStrList = logStr.split(QRegExp("\\s+"));

    for (int i = 0; i < logTableSize; i++) {
        // i+1 due to above split causing a blank first entry
        dblLogArray[i] = logStrList.at(i+1).toDouble();
    }
    dblLogArray[0] = 0;
    for (int i = 0; i < logTableSize; i++) {
        QCOMPARE((unsigned short)dblLogArray[i], uut.logTable[i]);
    }
}

void TestDSP::testRescale(void)
{
    DSP uut;
    unsigned short inputData[3] = { 0, 100, 1000};
    unsigned short outputData[3];
    double testWholeSamples[3] = { 1.0, 1.0, 2.0};
    double testFractionalSamples[3] = { 0.0, 0.5, 0.0 };
    uut.init();

    // Check sanity
    QCOMPARE(uut.rescale(0, 0, 0, outputData, 0), (unsigned int)0);
    QCOMPARE(uut.rescale(0, inputData, 0, 0, 0), (unsigned int)0);
    QCOMPARE(uut.rescale(0, 0, 0, 0, 0), (unsigned int)0);

    // Check a simple case, two point rescaling input, with a new point in between
    uut.wholeSamples = testWholeSamples;
    uut.fractionalSamples = testFractionalSamples;

    // Resulting value should be the midpoint of 100...1000 (since the sample number is 1.5), ie 550.
    uut.rescale(3, inputData, 3, outputData, 0);
    QCOMPARE(outputData[1], (unsigned short)550);

    // Don't allow destructor to delete these.
    uut.wholeSamples = NULL;
    uut.fractionalSamples = NULL;
}

void TestDSP::testEncoder(void)
{
    DSP *uut;
    int result;
    unsigned short signalBuffer[2048];
    unsigned short checkVal;

    uut = new DSP();
    uut->init();

    // Force these values to exercise the calibrator
    uut->encoderSettings.calMinVal = 1024;
    uut->encoderSettings.calMaxVal = 4096;

    //qDebug() << "testEncoder: test 1";
    // Test 1: Simply run through an increasing series of flat data
    // and check that the result makes sense.
    for (unsigned short val = 0; val <= 8192; val += 128)
    {
      for (int i = 0; i < 2048; i++)
      {
          signalBuffer[i] = val;
      }

      result = uut->computeEncoderAngle(signalBuffer, 2048);

      // Precalibration:
      if (val < 4096 ) 
      {
          checkVal = (double)uut->encoderSettings.linesPerRev * ( ( (double)val - 0.0 + 1 ) / ( 4096.0 - 0.0 + 2) );
          QCOMPARE((unsigned short)result, (unsigned short)(checkVal % uut->encoderSettings.linesPerRev));
      } 
      else 
      {
          // During calibration, the value shouldn't change; it should stay the maximum value
          QVERIFY( result == 1023 );
      }
      QVERIFY(result >= 0);
    }

    //qDebug() << "testEncoder: test 2";
    // Test 2: Run through again now that it has calibrated, expect the full range
    for (unsigned short val = 0; val <= 8192; val += 128)
    {
      for (int i = 0; i < 2048; i++)
      {
          signalBuffer[i] = val;
      }
      result = uut->computeEncoderAngle(signalBuffer, 2048);

      
      checkVal = (double)uut->encoderSettings.linesPerRev * ( ( (double)val - 0.0 + 1 ) / ( 8192.0 - 0.0 + 2) ); 
      QCOMPARE((unsigned short)result, (unsigned short)(checkVal % uut->encoderSettings.linesPerRev));
      QVERIFY(result >= 0);
    }

    // Test 3: Put in spiky data occasionally to see if it gets
    // filtered out
    //qDebug() << "testEncoder: test 3";
    int spikeCounter(10);
    for (unsigned short val = 0; val <= 8192; val += 128)
    {
      bool doingSpike(false);
      for (int i = 0; i < 2048; i++)
      {
          signalBuffer[i] = val;
      }

      if (spikeCounter-- == 0) 
      {
          signalBuffer[0] = 0;
          signalBuffer[2047] = 8192;
          doingSpike = true;
          spikeCounter = 10;
      }
      result = uut->computeEncoderAngle(signalBuffer, 2048);

      // If spiking, don't update checkVal, expect the last value.
      if (!doingSpike) 
      {
        checkVal = (double)uut->encoderSettings.linesPerRev * ( ( (double)val - 0.0 + 1 ) / ( 8192.0 - 0.0 + 2) ); 
      }
      QCOMPARE((unsigned short)result, (unsigned short)(checkVal % uut->encoderSettings.linesPerRev));
      QVERIFY(result >= 0);
    }

    delete uut;
}

void TestDSP::testTransform(void)
{
    DSP uut;
    QFile transformFile(transformTableName);
    uut.init();

    if (!transformFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QFAIL("Unable to load transform table file");
        return;
    }

    QTextStream s(&transformFile);

    unsigned short dataArray[freqTableSize];
    unsigned short transformArray[freqTableSize];
    double differenceArray[freqTableSize];
    double sumOfDiff(0);
    double sumOfSignal(0);
    double error(0);

    for (int i = 0; i < freqTableSize; i++) {

        /*
         * The transform table is arranged as alternating lines:
         * the first is the frequency table
         * the second is the result table
         */
        QString freqStr = s.readLine();
        QStringList freqStrList = freqStr.split(QRegExp("\\s+"));
        for (int j = 0; j < freqTableSize; j++) {

            // the first entry of the string
            // list is whitespace, hence the j+1
            dataArray[j] = (unsigned short)(freqStrList.at(j+1).toInt());
        }
        uut.transformData(dataArray);

        QString resultStr = s.readLine();
        QStringList resultStrList = resultStr.split(QRegExp("\\s+"));
        for (int j = 0; j < transformLength; j++) {

            // the first entry of the string
            // list is whitespace, hence the j+1
            transformArray[j] = resultStrList.at(j+1).toDouble();
        }

        for (int j = 0; j < freqTableSize; j++) {
            sumOfSignal += dataArray[j];
        }

        for (int j = 0; j < transformLength; j++ ) {
            differenceArray[j] = fabs(((double)transformArray[j] - (double)dataArray[j]));
            sumOfDiff += differenceArray[j];
        }

        error = sumOfDiff / sumOfSignal;
        sumOfDiff = 0;
        sumOfSignal = 0;

        // Check that the integrated error is less than 5% of the total signal
        //  qDebug() << "Error: " << error;
        QVERIFY(error < 0.05);

    }
}

QTEST_MAIN(TestDSP)
#include "main.moc"
