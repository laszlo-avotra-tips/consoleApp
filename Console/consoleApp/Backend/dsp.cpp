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
#include "signalmodel.h"


/*
 * constructor
 *
 * Common data for all DSPs
 */
DSP::DSP()
{
    // default service date
    serviceDate = QDate( 2009, 1, 1 );

    // frame data
    timeStamp    = QDateTime::currentDateTime().toUTC().toTime_t();
    milliseconds = QTime::currentTime().msec();
}


/*
 * init
 *
 * Initialize the common DSP data for use.  Data structures are set up
 * and the laser rescaling values are loaded from disk.
 */
void DSP::init()
{
    loadRescalingData();
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
    const unsigned int RescalingDataLength{2048};
    const QString StrRescalingData = SystemDir + "/RescalingData.csv";
    LOG1(StrRescalingData)

    QFile *input = new QFile( StrRescalingData );

    if( !input )
    {
        // fatal error
        LOG(INFO,"Could not create QFile to load rescaling data" )
        return;
    }

    if( !input->open( QIODevice::ReadOnly ) )
    {
        QString errString = QString( "Could not open " ) + StrRescalingData;
        LOG(INFO, errString )
    }

    QTextStream in( input );
    QString currLine = "";

    // Make sure this is a valid rescaling data file
    if( !findLabel( &in, &currLine, "% Laser Rescaling Data" ) )
    {
        emit LOG(INFO, "Laser Rescaling Data file is missing proper header." )
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
            LOG( INFO,  QString("The laser required service on %1.  Please contact Service." ).arg( serviceDate.toString() ) )
        }
        else if( NumDays < NumDaysToWarnForService )
        {
            LOG( INFO,  QString("The laser will require service by %1.  Please contact Service soon." ).arg( serviceDate.toString() ) )
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
                LOG( INFO,  QString( "Laser configuration file: Less data found than expected. Imaging may not work." ) )
                break;
            }
            else
            {
                currLine = in.readLine();
            }
        }
    }
    else
    {
        LOG( INFO,  QString( "Laser configuration file: No data found. Imaging may not work." ) )
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
    quint32 recordLength{DaqSettings::Instance().getRecordLength()};

    size_t average = 0;
    const quint32 start_idx = recordLength / 3 ;
    const quint32 end_idx   = ( 2 * recordLength ) / 3;

    for( quint32 i = start_idx; i < end_idx; i++ )
    {
        average += pA [ i ];
    }

    average /= ( recordLength / 3 );

    // XXX: ippsMean_16s_Sfs( (Ipp16s *)pA, (int)( recordLength/3 ), (Ipp16s *)&avg, 0 );

    return quint32(average);
}

unsigned int DSP::getTimeStamp() {
    return timeStamp;
}

int DSP::getMilliseconds() {
    return milliseconds;
}
