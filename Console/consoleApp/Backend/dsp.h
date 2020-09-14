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

#include <QTime>
#include "octFile.h"


class DSP
{
public:
    DSP();

    // prevent access to copy and assign
    DSP( DSP const & ) = delete;
    DSP & operator=( DSP const & ) = delete;

    void init();

    quint32 getAvgAmplitude( quint16 *pA );

    unsigned int getTimeStamp( void );
    int getMilliseconds( void );

private:
    void loadRescalingData( void );
    bool findLabel( QTextStream *in, QString *currLine, const QString Label );

    enum ExpectedSDKVersion
    {
        Major = 6,
        Minor = 0
    };

    // The service date of the console is associated with the configuration of the
    // laser.  The service date is stored in the laser configuration file.  This
    // date is read and checked when the rescaling values are read from disk.
    static const int NumDaysToWarnForService = 30;

    QDate  serviceDate;

    // Data related to the current A-line
    int milliseconds;
    unsigned int   timeStamp;
};


#endif // DSP_H_
