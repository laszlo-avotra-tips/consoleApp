/*
 * SoftwareDSP.h
 *
 * Interface for the CPU-based digital signal processing functions.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#ifndef SOFTWAREDSP_H
#define SOFTWAREDSP_H

#include "dsp.h"
#include "buildflags.h"
#include "ipp.h"

/*
 * Uses the CPU for DSP calculations
 */
class SoftwareDSP : public DSP
{
    Q_OBJECT

public:
    SoftwareDSP();
    ~SoftwareDSP();

    void init(unsigned int inputLength,
               unsigned int frameLines,
               unsigned int inBytesPerRecord,
               unsigned int inBytesPerBuffer,
               int inChannelCount );

    unsigned int rescale( const unsigned short *inputData );
    void processData( void );
    bool transformData( unsigned char *data );

public slots:
    void setEncoderConfigurationToDefault( void );

#if ENABLE_IPP_FFT_TUNING
    void setMagScaleFactor( int );
    void setFftScaleFactor( int );
#endif

#if ENABLE_LOW_SPEED_DATA_SNAPSHOT
    void saveSignals();
#endif

private:
    struct GearedAngle_T
    {
        int angle_cnt;
        int offset_cnt;
        int rawAngle_cnt;
    };

    // encoder data
    struct Encoder_t
    {
        unsigned short calMinVal;
        unsigned short calMaxVal;
        unsigned short gearingRatio; // encoder:catheter ratio
        unsigned short linesPerRev;
    };

    // IPP FFT data structure
    struct fftData_t
    {
        // FFT Control settings and buffers
        IppsFFTSpec_R_16s *fftSpec;
        char              *fftWorkBuf;
        unsigned int      inputSize;
        unsigned int      fftOrder;
        unsigned int      fftPoints;
        unsigned short    *fftChABuf;
        int               magScaleFactor;
        int               fftScaleFactor;
    };

    // used to determine encoder min and max values for this session
    static const int MaxEncoderChecks = 4;

    // Software DSP (low speed support)
    fftData_t fftData;

    // display computations
    unsigned short *adjustBuffer;
    unsigned short *resampledData;

    // encoder data
    Encoder_t encoderSettings;
    Encoder_t defaultEncoderSettings;
    GearedAngle_T currAngle;
    GearedAngle_T prevAngle;
    int numTimesWrapped;
    bool logEncoderValues;

    // encoder velocity data
    QTime encoderTimer;
    int prevTime;
    int currTime;

    // Lookup table for fast 16-bit int to scaled 16-bit int logarithm
    unsigned short *logTable;
    quint16 *m_daqRawData[2];

    int  computeEncoderAngle( unsigned short *data, int length );
    void checkInitFFTStructs( void );
    void computeLogTable( void );
    bool transformData_sw( unsigned short *data );

    bool adjustForDisplay( const unsigned short * const pFftData,
                           unsigned char * pDisplayData,
                           const int BufferSize );
};

#endif // SOFTWAREDSP_H
