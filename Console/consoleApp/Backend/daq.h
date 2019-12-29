/*
 * daq.h
 *
 * Interface to the AlazarTech DAQ hardware.  Creates a thread for collecting
 * and processing data from the DAQ hardware.
 *
 * This is a pure virtual class.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#ifndef DAQ_H_
#define DAQ_H_

//#include <QThread>
#include <QTime>
#include "AlazarApiType.h"
#include "buildflags.h"
#include "idaq.h"
#include <QMutex>

// Macro for the configure() function
#define CHECK_ERROR  if( status != ApiSuccess ){ shutdown(); return false; }

class DSP;

class DAQ : public IDAQ
{

public:
    DAQ();
    ~DAQ();

    // Static access allows checks without instantiating
    static bool checkSDKVersion( void );
    static bool checkDriverVersion( void );
    static bool isDaqPresent( void );
    static int  queryBoards( int numOfBoards );

//    RETURN_CODE AlazarGetChannelInfo( HANDLE h, U32 *MemSize, U8 *SampleSize);
    bool isApiSuccessChannelInfo(QString& msg, U32* msize, U8* sampleSize);

    enum Channels
    {
        ChannelA = 0,  // index into array, must be zero-based
        ChannelB = 1
    };

    virtual void init( void ) override;
    void run( void ) override;

    QString getDaqLevel( void ) override;
    void stop( void ) override;
    void pause( void ) override;
    void resume( void ) override;

    void enableAuxTriggerAsTriggerEnable( bool state ) override; //  * R&D only
    long getRecordLength() const override;


protected:
    // Alazar Channel Status (not defined in the Alazar headers)
    enum
    {
        Triggered     = 0x00000001,
        Saturated_ChA = 0x00000002,
        Saturated_ChB = 0x00000004,
        Error         = -1
    };

    enum ExpectedVersions  // Revision level is not checked; any revision will do
    {
        SDK_Major       = 5,
        SDK_Minor       = 9,
        Driver_Major    = 5,
        Driver_Minor    = 9,
        FPGA_Major      = 35,
        FPGA_Minor      = 8
    };

    bool isConfigured;
    bool isRunning;

    BoardDef bd;
    HANDLE   hBoard;
    U32      recordToRead; // Which record to read from the buffer (if there are more than one)

    DSP  *dsp;

    unsigned short linesPerRevolution;

    // Async DMA variables
    U32 recordsPerBuffer;
    U32 bytesPerRecord;
    U32 bytesPerBuffer;
    int channelCount;
    int frameCount;

    QTime triggerTimer;
    QMutex gFrameWriteMutex;


//    virtual bool configure( void ) = 0;
//    virtual bool getData( void ) = 0;

    void shutdown( void );

    // prevent access to copy and assign
    DAQ( DAQ const & );
    DAQ & operator=( DAQ const & );
};


#endif // DAQ_H_
