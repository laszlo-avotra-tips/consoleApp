/*
 * dsp.cpp
 *
 * Handle raw signal data from the DAQ
 *    - Resample raw laser data
 *    - Process A-lines
 *    - Encoder angle
 *    - Scale output for display
 *
 * The input data is expected to be in two arrays of double precision, the first
 * are the whole parts of the sample numbers appearing at linear frequency intervals.
 * The second contains the fractional parts. The code then (linearly) interpolates the data
 * values at the whole + fractional sample number based on the preceding and following
 * whole samples.
 *
 * Author(s): Dennis W. Jackson, Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "defaults.h"
#include "dsp.h"
#include "logger.h"
#include "math.h"
#include "octFile.h"
#include "profiler.h"
#include "sawFile.h"
#include "daqSettings.h"
#include "deviceSettings.h"
#include "util.h"
#include "buildflags.h"
#include "depthsetting.h"
#include "Utility/userSettings.h"


/*
 * constructor
 *
 * Common data for all DSPs
 */
DSP::DSP()
{
    // default service date
    serviceDate = QDate( 2009, 1, 1 );

    isRunning = false;

    deviceSettings &dev = deviceSettings::Instance();
    aLineLength_px = dev.current()->getALineLengthDeep_px(); // Always use 1024 depth line, is this a problem in Low Speed?

    // frame data
    timeStamp    = QDateTime::currentDateTime().toUTC().toTime_t();
    milliseconds = QTime::currentTime().msec();

    // Contrast stretch defaults (no stretch)
    blackLevel = BrightnessLevels_HighSpeed.defaultValue;
    whiteLevel = ContrastLevels_HighSpeed.defaultValue;

    bytesPerBuffer = 0;
    bytesPerRecord = 0;
}

/*
 * Destructor
 *
 * Release memory
 */
DSP::~DSP()
{
    qDebug() << "DSP::~DSP()";
    LOG( INFO, "DSP shutdown" )

    if( fractionalSamples )
    {
        delete [] fractionalSamples;
    }

    if( wholeSamples )
    {
        delete [] wholeSamples;
    }
}


/*
 * init
 *
 * Initialize the common DSP data for use.  Data structures are set up
 * and the laser rescaling values are loaded from disk.
 */
void DSP::init( unsigned int inputLength,
                unsigned int frameLines,
                int inBytesPerRecord,
                int inBytesPerBuffer
                )
{
    recordLength   = inputLength;
    linesPerFrame  = frameLines;
    bytesPerRecord = quint32(inBytesPerRecord);
    bytesPerBuffer = quint32(inBytesPerBuffer);

    qDebug() << "DSP: linesPerFrame"  << linesPerFrame;
    qDebug() << "DSP: recordLength"   << recordLength;
    qDebug() << "DSP: bytesPerRecord" << bytesPerRecord;
    qDebug() << "DSP: bytesPerBuffer" << bytesPerBuffer;

    useDistalToProximalView = true;
    doInvertColors          = false;

    // Memory for the rescaling data
    wholeSamples      = new float[ RescalingDataLength ];
    fractionalSamples = new float[ RescalingDataLength ];

    if( !wholeSamples || !fractionalSamples )
    {
        // fatal error
        displayFailureMessage( tr( "Could not allocate memory for the rescaling values" ), true );
    }

    // populate wholeSamples[] and fractionSamples[]
//    LOG2(recordLength,linesPerFrame);
//    LOG2(bytesPerRecord,bytesPerBuffer);
    loadRescalingData();

    // Update radius and offset, set to Normal Mode upon new device select
    deviceSettings &settings = deviceSettings::Instance();
    catheterRadius_px = float(settings.current()->getCatheterRadius_px());
    catheterRadius_um = float(settings.current()->getCatheterRadius_um());
    internalImagingMask_px = float(settings.current()->getInternalImagingMask_px());
}

/*
 * run
 *
 * Data processing thread.  This is the main loop for the DSP.
 */
void DSP::run( void )
{
    qDebug() << "DSP::run start";

    // prevent multiple, simultaneous starts
    if( !isRunning )
    {
        qDebug() << "Thread: DSP::run start";
        isRunning = true;

        while( isRunning )
        {
            // run full-tilt.
            processData();
        }

        qDebug() << "Thread: DSP::run stop";
    }
}


/*
 * loadRescalingData
 *
 * The rescaling file is created by the OCT Laser Configuration tool at
 * manufacturing/service.  The service date is loaded from the header to
 * determine if service is due soon.  The rescaling data for the waveform
 * is contained in the DATA section of the file.  The rest of the header
 * is for internal use only.
 */
void DSP::loadRescalingData( void )
{
    // Load the configuration file
    const QString StrRescalingData = SystemDir + "/RescalingData.csv";
    LOG1(StrRescalingData)

    QFile *input = new QFile( StrRescalingData );

    if( !input )
    {
        // fatal error
        emit sendError( tr( "Could not create QFile to load rescaling data" ) );
        return;
    }

    if( !input->open( QIODevice::ReadOnly ) )
    {
        QString errString = tr( "Could not open " ) + StrRescalingData;
        emit sendError( errString );
    }

    QTextStream in( input );
    QString currLine = "";

    // Make sure this is a valid rescaling data file
    if( !findLabel( &in, &currLine, "% Laser Rescaling Data" ) )
    {
        emit sendError( tr( "Laser Rescaling Data file is missing proper header." ) );
    }
    // Grab the service date so the upper levels can access it
    if( findLabel( &in, &currLine, "% Last Service Date" ) )
    {
        QString strDate = currLine.section( ":", 1, 1 ).trimmed();
        serviceDate = QDate::fromString( strDate, "yyyy-MM-dd" );
        LOG( INFO, "Last service date: " + serviceDate.toString() )

        // set date service is due
        serviceDate = serviceDate.addYears( 1 );
        
        const QDate Now = QDate::currentDate();
        const int NumDays = int(Now.daysTo( serviceDate ));

        // If the service date has passed (negative number of days), warn the operator
        // but let the application run. If the service date is within 30 days of expiring
        // pop up a warning message at start up.
        if( NumDays < 0 )
        {
            emit sendWarning( QString( tr( "The laser required service on %1.  Please contact Service." ) ).arg( serviceDate.toString() ) );
        }
        else if( NumDays < NumDaysToWarnForService )
        {
            emit sendWarning( QString( tr( "The laser will require service by %1.  Please contact Service soon." ) ).arg( serviceDate.toString() ) );
        }
    }
    // Find and load the rescaling data
    if( findLabel( &in, &currLine, "% DATA" ) )
    {
        // Load the data into the arrays
        for( unsigned int i = 0; i < RescalingDataLength; i++ )
        {
            /*
             * Make sure the amount of data expected is in the file. If not,
             * warn but allow the software to continue to run.
             */
            if( in.atEnd() && ( i < RescalingDataLength ) )
            {
                emit sendWarning( tr( "Laser configuration file: Less data found than expected. Imaging may not work." ) );
                break;
            }
            else
            {
                currLine = in.readLine();
                wholeSamples[ i ]      = currLine.section( ",", 1, 1 ).toFloat();
                fractionalSamples[ i ] = currLine.section( ",", 2, 2 ).toFloat();
            }
        }
    }
    else
    {
        emit sendWarning( tr( "Laser configuration file: No data found. Imaging may not work." ) );
    }

    // free the pointer.  nullptr check done above.
    delete input;
}

/*
 * findLabel
 *
 * Find sections of the rescaling data file for additional processing. The last line
 * read is returned to the calling function.
 * 
 */
bool DSP::findLabel( QTextStream *in, QString *currLine, const QString Label )
{
    bool foundLabel = false;

    do
    {
        *currLine  = in->readLine();
        foundLabel = currLine->contains( Label );
    } while( !foundLabel && !in->atEnd() );

    return foundLabel;
}

/*
 * getAvgAmplitude()
 *
 * Calculate the average amplitude of an A-line using rawData between the first
 * third and second third samples of the A-line. This is called by
 * setEvoaPowerLevel() to read A-line strength and determine adjustment amount.
 */
quint32 DSP::getAvgAmplitude( quint16 *pA )
{
    quint32 average = 0;
    const int start_idx = recordLength / 3 ;
    const int end_idx   = ( 2 * recordLength ) / 3;

    for( int i = start_idx; i < end_idx; i++ )
    {
        average += pA [ i ];
    }

    average /= ( recordLength / 3 );

    // XXX: ippsMean_16s_Sfs( (Ipp16s *)pA, (int)( recordLength/3 ), (Ipp16s *)&avg, 0 );

    return average;
}

void DSP::updateCatheterView()
{
    userSettings &user = userSettings::Instance();
    useDistalToProximalView = user.isDistalToProximalView();
}
