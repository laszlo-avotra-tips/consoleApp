/*
 * dsp.h
 *
 * Interface for the digital signal processing functions.
 *
 * This is a pure virtual class.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#ifndef DSP_H_
#define DSP_H_

#include <QThread>
#include <QTime>
#include "octFile.h"
#include "buildflags.h"

#ifdef UNIT_TEST
#define private public
#endif


class DSP : public QThread
{
    Q_OBJECT

public:
    DSP();
    virtual ~DSP();

    static bool checkIPPVersion( void );

    virtual void init( unsigned int inputLength,
                       unsigned int frameLines,
                       int inBytesPerRecord,
                       int inBytesPerBuffer,
                       int inChannelCount
                       );

    virtual void run( void );
    virtual void processData( void ) = 0;

    quint32 getAvgAmplitude( quint16 *pA );

    // The size of the rescaling data is constant for all lasers
    static const unsigned int RescalingDataLength = 2048;

signals:
    void sendWarning( QString );
    void sendError( QString );

public slots:
    void setBlackLevel( int val ) { blackLevel = val; }
    void setWhiteLevel( int val ) { whiteLevel = val; }
    void stop( void ) { isRunning = false; }
    void updateCatheterView();
//    {
//        userSettings &user = userSettings::Instance();
//        useDistalToProximalView = user.isDistalToProximalView();
//    }

    void setInvertColors( bool enable ) { doInvertColors = enable; }

#if ENABLE_RAW_DATA_SNAPSHOT
    void recordRawData( int rawDataLength );
    void closeRawDataFiles( void );
#endif

#if CONSOLE_MANUFACTURING_RELEASE
    void enableOcelotSwEncoder( bool enabled )
    {
        useSwEncoder = enabled;
    }
#endif

protected:
    virtual unsigned int rescale( const unsigned short *inputData ) = 0;

    bool useDistalToProximalView;
    bool doInvertColors;

    // 8-bit full-range value (2^8 - 1)
    static const unsigned short FullScaleIntensityValue = 255;

private:
    enum ExpectedSDKVersion
    {
        Major = 6,
#ifdef WIN32
        Minor = 0
#else
        Minor = 1
#endif
    };

    // The service date of the console is associated with the configuration of the
    // laser.  The service date is stored in the laser configuration file.  This
    // date is read and checked when the rescaling values are read from disk.
    static const int NumDaysToWarnForService = 30;

    bool isRunning;

protected:
    int channelCount;

    // pointer into the global data structure for passing frame data around
    OCTFile::FrameData_t *pData;

#if ENABLE_FRAME_COUNTERS_TO_DEBUG
    QTime frameTimer;
#endif

    unsigned int bytesPerRecord; // sending raw data to frontend
    unsigned int bytesPerBuffer; // working buffer space alloc
    unsigned int recordLength;   // Single A-Line sampled data length
    unsigned int linesPerFrame;  // Number of lines in a frame to operate on at once
    int aLineLength_px;

    // rescaling data
    float *wholeSamples;
    float *fractionalSamples;
    QDate  serviceDate;

    float blackLevel;  // i.e., brightness
    float whiteLevel;  // i.e., contrast
    float internalImagingMask_px;
    float catheterRadius_px;
    float catheterRadius_um;

    // Data related to the current A-line
    unsigned int   timeStamp;
    int milliseconds;

    unsigned int getTimeStamp( void ) { return timeStamp; }
    int getMilliseconds( void ) { return milliseconds; }

    void loadRescalingData( void );
    bool findLabel( QTextStream *in, QString *currLine, const QString Label );

#if ENABLE_RAW_DATA_SNAPSHOT
    QFile       *hRawFile;
    QDataStream  rawOutputStream;
    bool         isRecordRaw;
    int          rawCount;
    int          snapshotLength;
    QFile       *hRawTextFile;
    QTextStream  outputTextStream;
#endif

#if CONSOLE_MANUFACTURING_RELEASE
    bool useSwEncoder;
#endif

private:
    // Code for development without a GPU
    // void postProc_sw( float *input_re, float *input_imag,  unsigned short *output );

    // prevent access to copy and assign
    DSP( DSP const & );
    DSP & operator=( DSP const & );
};


#endif // DSP_H_
