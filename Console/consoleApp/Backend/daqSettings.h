/*
 * daqSettings.h
 *
 * Description: Load and store in memory settings that are related to the
 *              DAQ hardware.  This object uses the singleton pattern to
 *              allow it to be accessed from multiple locations in the codebase.
 *              It is a read-only class.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#ifndef DAQSETTINGS_H_
#define DAQSETTINGS_H_

#include <QSettings>

// ATS DAQ records must be aligned at specific boundaries. Records may shift within
// a buffer if alingment requirements are not met.
//const int DaqRecordSampleAlignment = 32;  // ATS9462

class DaqSettings
{

public:
    // Singleton
    static DaqSettings & Instance();

    bool init( void );
    void load( void );

    unsigned long  getRecordCount( void ) { return recordCount; }
    unsigned long  getPreDepth( void ) { return preDepth; }
    unsigned long  getRecordLength( void ) { return recordLength; }
    unsigned long  getChAInputRange( void ) { return chAInputRange; }
    unsigned long  getChBInputRange( void ) { return chBInputRange; }
    // TBD - may want to move encoder stuff to the device level
    unsigned short getEncoderCalMin( void ) { return encoderCalMin; }
    unsigned short getEncoderCalMax( void ) { return encoderCalMax; }
    unsigned short getEncoderMaxPosition( void ) { return encoderMaxPosition; } //XXX hmmm....

private:
    DaqSettings();  // hide ctor
    ~DaqSettings(); // hide dtor
    DaqSettings( DaqSettings const & ); // hide copy
    DaqSettings & operator=( DaqSettings const & ); // hide assign

    QSettings *settings;
    QString    defaultPath;
    QString    basename;

    unsigned long  recordCount;
    unsigned long  preDepth;
    unsigned long  recordLength;
    unsigned long  chAInputRange;
    unsigned long  chBInputRange;
    unsigned short encoderCalMin;
    unsigned short encoderCalMax;
    unsigned short encoderMaxPosition;
    unsigned short encoderCounts;

    bool isReady;
    const ulong DaqRecordSampleAlignment {32};  // ATS9462
    static DaqSettings* theSettings;

};

#endif  // DAQSETTINGS_H_

