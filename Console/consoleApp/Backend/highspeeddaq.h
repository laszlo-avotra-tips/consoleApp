///*
// * highspeeddaq.h
// *
// * Implements the DAQ interface for high speed devices. High speed devices
// * acquire and process an entire image worth of A-lines at a time and
// * provide the fully baked image to the frontend.
// *
// * Author: Dennis W. Jackson
// *
// * Copyright (c) 2011-2018 Avinger, Inc.
// *
// */
//#include "buildflags.h"

//#ifndef HIGHSPEEDDAQ_H
//#define HIGHSPEEDDAQ_H

//#include "daq.h"

//#if _DEBUG || ENABLE_LAPTOP_MODE
//#   define SYNTHDATA 1
//#else
//#   define SYNTHDATA 0
//#endif
//#define SYNTHETIC_DATA_BUFFERS 5

///*
// * HighSpeedDAQ
// */
//class HighSpeedDAQ : public DAQ
//{
//public:
//    HighSpeedDAQ();
//    ~HighSpeedDAQ();
//    void init( void );
//    bool configure( void );
//    bool getData( void );

//private:
//    int buffersPosted;
//    int bufferIndex;
//    int buffersCompleted;
//    int timeoutCounter;

//    // prevent access to copy and assign
//    HighSpeedDAQ( HighSpeedDAQ const & );
//    HighSpeedDAQ & operator=( HighSpeedDAQ const & );

//    // Buffers for DMA transfers. More buffers allows the OS to go off and do other tasks
//    // without risking the DAQ HW overrunning already-filled DMA areas.
//    U16 *captureBuffer[ 16 ];  // XXX: bigger here with lower RPM makes malloc unhappy allocating captureBuffers[] -- heap issue

//#if SYNTHDATA
//    // Data for testing without a DAQ at high speed
//    void buildSyntheticData();
//    U16 *syntheticData[ SYNTHETIC_DATA_BUFFERS ];
//#endif

//};

//#endif // HIGHSPEEDDAQ_H


