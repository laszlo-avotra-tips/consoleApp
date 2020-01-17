/*
 * dataConsumer.cpp
 *
 * Gets data from the shared memory pool.  The DAQ thread populates the memory,
 * this object uses it.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */

#include <QDebug>
#include "daqDataConsumer.h"
#include "defaults.h"
#include "logger.h"
#include "profiler.h"
#include "buildflags.h"
#include "daqSettings.h"
#include "deviceSettings.h"
#include "userSettings.h"
#include "theglobals.h"

// shared data with DAQ thread
const int FullCaseVideoWidth_px( 1024 );
const int FullCaseVideoHeight_px( int(double( SectorHeight_px + WaterfallHeight_px ) * double(FullCaseVideoWidth_px) / double(SectorWidth_px) ) );

const int LoopVideoWidth_px( 1024 );
const int LoopVideoHeight_px( int(double( SectorHeight_px + WaterfallHeight_px ) * double(LoopVideoWidth_px) / double(SectorWidth_px ) ) );

const int ThreadWaitTimeout_ms = 5000;  // XXX: Make sure the video threads exit.  Would prefer to know why they sometimes do not

/*
 * constructor
 */
DaqDataConsumer::DaqDataConsumer( liveScene *s,
                                  advancedView *adv,
                                  EventDataLog *eLog )
{
    sceneInThread        = s;
    advViewInThread      = adv;
    eventLog             = eLog;
    pData                = nullptr;
    currFrame            = 0;
    prevFrame            = 0;
    isRunning            = false;
    isRecordFullCaseOn   = true;
    frameCount           = 0;
    millisecondsPerFrame = 0.0;

    prevRecordClip = false;
    recordClip     = false;

    strFullCaseNumber = "";

    clipFile       = "";
    clipFrameCount = 0;
    lastTimestamp  = 0;
    clipEncoder    = nullptr;
    caseEncoder    = nullptr;

    prevDirection = directionTracker::Stopped;
    currDirection = directionTracker::Stopped;
    videoEncoder::initVideoLibrary();

    useDistalToProximalView = true;
    processingTimer.start();

    safeFrameBuffer = static_cast<char*>(malloc( ( SectorHeight_px + WaterfallWidth_px ) * SectorWidth_px ) );

    /*
     * Get the state of the Full Case recording flag from the system settings. The
     * default is to always enable Full Case recording.
     */
    systemSettings = new QSettings( SystemSettingsFile, QSettings::IniFormat );

#if DISABLE_BACKGROUND_RECORDING
    isAlwaysRecordFullCaseOn = false;
#else
    if( systemSettings )
    {
        isAlwaysRecordFullCaseOn = systemSettings->value( FullCaseRecordingSetting, true ).toBool();
    }
    else
    {
        isAlwaysRecordFullCaseOn = true;
    }
#endif
}

/*
 * destructor
 */
DaqDataConsumer::~DaqDataConsumer()
{
    // Close out the current full case recording
    if( caseEncoder )
    {
        caseEncoder->close();
        caseEncoder->wait( ThreadWaitTimeout_ms );
        delete caseEncoder;
        caseEncoder = nullptr;
    }

    if( systemSettings )
    {
        delete systemSettings;
        systemSettings = nullptr;
    }
}

/*
 * DaqDataConsumer::run
 *
 * Thread to handle getting data from the DAQ thread.
 *
 */
void DaqDataConsumer::run( void )
{
    qDebug() << "Thread: frontend::DaqDataConsumer::run()";
    LOG( INFO, "DaqDataConsumer Thread started" )

    // access lines per revolution
    deviceSettings &devSettings = deviceSettings::Instance();
    dirTracker.getEncoderCountsForDevice(); // a new device is selected, find its encoder range.

    // Notify Advanced View if full case is being recorded
    emit alwaysRecordingFullCase( isAlwaysRecordFullCaseOn );
    LOG( INFO, QString( "Full case recording: %1" ).arg( isAlwaysRecordFullCaseOn ) )

    // initialize prevFrame to the same value that currFrame will get so no
    // work is done until the DAQ and DSP start up
    prevFrame = TheGlobals::instance()->getPrevGFrameCounter();

    timeoutCounter = 0;
    isRunning = true;
    while( isRunning )
    {
        TIME_THIS_SCOPE( ddc_run_all );

        /*
         * DDC is not shut down between device changes so each frame
         * needs to get the latest device information.
         */
        bool isHighSpeedDevice  = devSettings.current()->isHighSpeed();

        // wrap-safe method to grab one index behind the frame that is currently being filled
        currFrame = TheGlobals::instance()->getPrevGFrameCounter();

        // If we have a new frame, process it.
        if( currFrame != prevFrame )
        {
            TIME_THIS_SCOPE( ddc_run );
            timeoutCounter = HsVideoTimeoutCount;

            // grab the data for this frame
            pData = TheGlobals::instance()->getFrameDataPointer(currFrame);

            if( advViewInThread->isVisible() )
            {
                /*
                 *  Send raw and FFT data to the Advanced View plots. This MUST
                 *  be done via signal or the shared pointer does strange things.
                 */
                emit updateAdvancedView( pData );
            }

            // copy frame data into the shared pointer for this line
            QSharedPointer<scanframe> frame = QSharedPointer<scanframe>( new scanframe() );

            // fill in data to send out to other frontend objects
            if( isHighSpeedDevice )
            {
                frame->angle = 0;                // no angle data in high speed devices
                frame->depth = SectorHeight_px;
                frame->width = SectorWidth_px;
            }
            else
            {
                frame->angle = pData->encoderPosition;
                frame->depth = 1;
                frame->width = 512;
            }
            frame->frameCount = pData->frameCount; // not used by frontend
            frame->dispData   = new QByteArray( reinterpret_cast<const char *>(pData->dispData), int(frame->depth * frame->width) );
            frame->videoData  = new QByteArray( reinterpret_cast<const char *>(pData->videoData), int(frame->depth * frame->width ) );
            frame->timestamp  = pData->timeStamp;

            // Add this line to the scene
            sceneInThread->addScanFrame( frame );

            // Optimize a bit, don't always need to be drawing the time
            if( lastTimestamp != pData->timeStamp )
            {
                lastTimestamp = pData->timeStamp;
            }

            // Monitor direction changes for Low Speed only
            if( !isHighSpeedDevice )
            {
                int  linesPerRevolution = devSettings.current()->getLinesPerRevolution();
                // determine the direction of rotation of the encoder and broadcast this information
                // This must undo the changes from the raw encoder that were added in by the DAQ
                // for point of view changes. Complement with max encoder value ( encoderCounts - 1 ).
                // TBD - may want to move encoder stuff to the device level
                if( useDistalToProximalView )
                {
                    dirTracker.updateDirection( quint16(linesPerRevolution - 1  - pData->encoderPosition ));
                }
                else
                {   // proximal to distal view
                    dirTracker.updateDirection( pData->encoderPosition );
                }
                currDirection = dirTracker.getDirection();

                // only signal the system when there is a change
                if( currDirection != prevDirection )
                {
                    emit directionOfRotation( currDirection );
                }
            }

            /*
             * Allow toggling of background recording on/off. Protect against trying to
             * turn on recording when it was requested before starting the thread
             */
            if( isRecordFullCaseOn )
            {
                // Update the frame count for lines consumed and saved
                frameCount++;

                // Update the original data for processing below
                pData->frameCount = frameCount;

                // handle any queued events
                if( !addEventQ.empty() )
                {
                    mutex.lock();
                    eventLog->addEvent( addEventQ.dequeue(),
                                        pData->frameCount,
                                        pData->timeStamp,
                                        QString( "" ) ); // XXX: event log may be removed
                    mutex.unlock();
                }
            }

            /*
             * Only do recording-related work if either an OCT Loop or Full Case recording is going on
             */
            if( ( recordClip && ( clipFile != nullptr ) ) || ( isRecordFullCaseOn && isAlwaysRecordFullCaseOn ) )
            {
                    // Don't do anything until the video encoder is ready
                    enum videoEncoder::videoStatus_e status = videoEncoder::Starting;

                    /*
                     * Make sure both encoders are ready, if they are available.
                     */
                    if( clipEncoder )
                    {
                        while ( ( status = clipEncoder->initComplete() ) == videoEncoder::Starting )
                        {
                            QApplication::processEvents();
                        }
                    }
                    if( caseEncoder )
                    {
                        while ( ( status = caseEncoder->initComplete() ) == videoEncoder::Starting )
                        {
                            QApplication::processEvents();
                        }
                    }

                    /*
                     * Send image data to the video encoder.
                     */
                    if( !isHighSpeedDevice )
                    {
                        /*
                         * Low Speed Device
                         */
                        if( frameTimer.elapsed() > millisecondsPerFrame )
                        {
                            // all the work of adding this frame to the video should not count against
                            // the timing of the next frame
                            frameTimer.restart();

                            // Memcpy is required to keep a consistent pointer for each frame that qt can't
                            // pull out from under the videoencoder thread.
                            sceneInThread->lockFrame();
                            memcpy( safeFrameBuffer, sceneInThread->frameSample(), SectorWidth_px * SectorHeight_px );
                            sceneInThread->unlockFrame();
                            sceneInThread->applyClipInfoToBuffer( safeFrameBuffer );
                            sceneInThread->wfSample( safeFrameBuffer + (SectorWidth_px * SectorHeight_px ) );
                            emit addFrame( safeFrameBuffer );
                        }
                    }
                    clipFrameCount++;
            }

            // prepare for the next iteration
            prevFrame      = currFrame;
            prevDirection  = currDirection;

#if ENABLE_FRAME_COUNTERS_TO_DEBUG
            // Rough frames/second counter  XXX
            static int dispFrameCount = 0;
            dispFrameCount++;

            if( processingTimer.elapsed() > 1000 )
            {
                qDebug() << "                                                     DDC frameCount/s:" << dispFrameCount;
                dispFrameCount = 0;
                processingTimer.restart();
            }
#endif
        }

        if( ( timeoutCounter > 0) && isHighSpeedDevice &&
                ( ( recordClip && ( clipFile != nullptr ) ) || ( isRecordFullCaseOn && isAlwaysRecordFullCaseOn ) ) )
        {
            /*
             * High Speed Device
             */
            if( frameTimer.elapsed() >= millisecondsPerFrame )
            {
                frameTimer.restart();
                timeoutCounter--;
                sceneInThread->applyClipInfoToBuffer( reinterpret_cast<char *>(pData->videoData ) );
                if( clipEncoder )
                {
                    clipEncoder->addFrame( reinterpret_cast<char *>(pData->videoData ) );
                }
                if( caseEncoder )
                {
                    caseEncoder->addFrame( reinterpret_cast<char *>(pData->videoData ) );
                }
            }
        }

        /*
         * If the clip recording has been turned off, signal
         * that all data has been written to the file. This
         * check is outside of the frame to frame check to ensure
         * clip recordings will stop when the high speed catheter
         * is not spinning (i.e., sending top dead center).
         */
        if( !recordClip && prevRecordClip )
        {
            emit clipRecordingStopped();
        }

        // prepare for the next iteration
        prevRecordClip = recordClip;

        yieldCurrentThread();
    }

    LOG( INFO, "DaqDataConsumer Thread stopped" )
    qDebug() << "Thread: frontend::DaqDataConsumer::stopped()";
}

/*
 * handleTagEvent
 *
 * Adds a tag to queue if data collection is ongoing.  The queue is processed
 * in main thread loop.
 */
void DaqDataConsumer::handleTagEvent( QString event )
{
    mutex.lock();
    addEventQ.enqueue( event );
    mutex.unlock();
}

/*
 * setupEncoder
 *
 * Video Encoder Setup.
 */
void DaqDataConsumer::setupEncoder( videoEncoder **cdc, const QString VidFilename, const int Width_px, const int Height_px, const bool IsRaw )
{
    double fps = 0.0;

    deviceSettings &devSettings = deviceSettings::Instance();
    bool isHighSpeedDevice = ( devSettings.current()->isHighSpeed() );

    if( isHighSpeedDevice )
    {
        fps = DefaultHsVideoFPS;
        millisecondsPerFrame = ( 1.0 / fps ) * 1000.0;
    }
    else
    {
        fps = DefaultLsVideoFPS;
        millisecondsPerFrame = ( 1.0 / fps ) * 1000.0;
    }
    qDebug() << "Starting new clip encoding at " << fps << "fps (millisecondsPerFrame" << millisecondsPerFrame << ")";

    if( isHighSpeedDevice )
    {
        *cdc = new videoEncoder( VidFilename.toLatin1().data(),
                                 SectorWidth_px,
                                 SectorHeight_px,
                                 Width_px,
                                 Width_px,   // 1:1 aspect ratio for high speed, no waterfall.
                                 fps );
    }
    else
    {
        // Include space for the waterfall XXX Where to get canonical wf size?
        *cdc = new videoEncoder( VidFilename.toLatin1().data(),
                                 SectorWidth_px,
                                 SectorHeight_px + WaterfallHeight_px,
                                 Width_px,
                                 Height_px,
                                 fps );
    }

    if( IsRaw )
    {
        connect( this, SIGNAL( rawVideoClose() ), *cdc, SLOT( close() ) ); // xxx: not used
    }
    else
    {
        connect( this, SIGNAL( videoClose() ), *cdc, SLOT( close() ) );
    }
    connect( this, SIGNAL( addFrame(char*) ), *cdc, SLOT( addFrame(char*) ) );
}

/*
 * startClipRecording
 *
 *
 */
void DaqDataConsumer::startClipRecording( void )
{
    clipFrameCount = 0;
    recordClip = true;
    timeoutCounter = 0;

    setupEncoder( &clipEncoder,
                  clipFile + LoopVideoExtension,
                  LoopVideoWidth_px,
                  LoopVideoHeight_px,
                  false );

    clipEncoder->start();
    frameTimer.start();
    qDebug() << " Started new clip encoding.";
}

/*
 * stopClipRecording
 *
 *
 */
void DaqDataConsumer::stopClipRecording( void )
{
    // tell the thread to stop saving clip data
    stop();
    wait( ThreadWaitTimeout_ms );
    qDebug() << "Stopping clip encoder.";
    clipEncoder->disconnect();

    recordClip = false;
    clipFile = "";
    if( clipEncoder )
    {
        emit videoClose();
        clipEncoder->wait( ThreadWaitTimeout_ms );
        
        // Pass the full video duration to Frontend to be written to the database.
        emit sendVideoDuration( clipEncoder->getDuration() );
        delete clipEncoder;
        clipEncoder = nullptr;
    }
    qDebug() << "Clip encoder finished.";
    start();  // XXX WTF?
}

/*
 * recordBackgroundData
 *
 * Sets up the background recording stream when enabled; otherwise
 * closes down the stream.
 */
void DaqDataConsumer::recordBackgroundData( bool state )
{
    // only record if home screen has it enabled
    if( isAlwaysRecordFullCaseOn )
    {
        isRecordFullCaseOn = state;

        if( state )
        {
            // set up file storage and start recording
            static int currFullCaseNumber = 0;
            currFullCaseNumber++;
            strFullCaseNumber = QString( "%1" ).arg( currFullCaseNumber, 3, 10, QLatin1Char( '0' ) );

            // Start up background recording
            timeoutCounter = 0;
            caseInfo       &ci = caseInfo::Instance();
            deviceSettings &ds = deviceSettings::Instance();
            QString caseRecordingName( ci.getFullCaseDir() + "/fullCase-" + strFullCaseNumber + CaseVideoExtension );
            QImage thumbnail = ds.current()->getIcon();
            thumbnail.save( ci.getFullCaseDir() + "/fullCase-" + strFullCaseNumber + "_thumb" + ".png" );
            setupEncoder( &caseEncoder, caseRecordingName, FullCaseVideoWidth_px, FullCaseVideoHeight_px, true );
            caseEncoder->start();
            frameTimer.start();

            LOG( DEBUG, QString( "Background Recording: Start (%1)" ).arg( strFullCaseNumber ) )
            qDebug() << QString( "Background Recording: Start (%1)" ).arg( strFullCaseNumber );
        }
        else
        {
            // Close the background recording if it is currently ongoing
            if( caseEncoder )
            {
                caseEncoder->close();
                caseEncoder->wait( ThreadWaitTimeout_ms );
                delete caseEncoder;
                caseEncoder = nullptr;

                LOG( DEBUG, QString( "Background Recording: Stop (%1)" ).arg( strFullCaseNumber ) )
                qDebug() << QString( "Background Recording: Stop (%1)" ).arg( strFullCaseNumber );
            }
        }
    }
}

/*
 * handleAutoAdjustBrightnessContrast
 *
 * This is a slot to handle the request from the auto-adjust pushbutton.
 * Takes the current line of processed data ( formatted for display ) and finds the
 * optimal brightness and contrast values.
 *
 * Algorithm: Ignore the points from 0 ~ gluelineOffset, because they are artificially strong in magnitude
 *            reflections or FFT artifacts. Set the brightness to the min value + some percentage
 *            ( RelaxationParameter ) of the range, and the contrast to the max value.
 */
void DaqDataConsumer::handleAutoAdjustBrightnessAndContrast( void )
{
    // Only perform the search if data is available
    if( pData )
    {
        const int MaxADCVal = 65535; // full range of 16-bit card
        deviceSettings &dev = deviceSettings::Instance();
        int maxVal = 0;
        int minVal = MaxADCVal; // max value of the processed data graph--2^8 for High Speed and 2^16 for low speed
        const float RelaxationParameter = 0.2f;

        /*
         * Search for min and max from the glueline offset to the Normal View pixel depth.
         * The processed data graph uses 1024 points, but we should only base this calculation on
         * the viewable points in the Normal View mode.
         */
        for( quint32 i = quint32(dev.current()->getInternalImagingMask_px() ); i < ( FFTDataSize / 2 ); i++ )
        {
            // find min
            if( pData->fftData[ i ] < minVal )
            {
                minVal = pData->fftData[ i ];
            }

            // find max
            if( pData->fftData[ i ] > maxVal )
            {
                maxVal = pData->fftData[ i ];
            }
        }

        // Set the new brightness at the minimum plus 20% of the range of max to min.
        int newBrightness = minVal + int(float( RelaxationParameter) * float( maxVal - minVal ) );

        // Adjustment algorithm: Set the Brightness and Contrast to Min and Max, respectively.
        emit updateBrightness( newBrightness );
        emit updateContrast(   maxVal );
    }
}

/*
 * updateCatheterView
 *
 * Update the member variable. Use a cached member variable for improved performance instead of making the
 * context change to userSettings::catheterView() within run().
 */
void DaqDataConsumer::updateCatheterView()
{
    userSettings &user = userSettings::Instance();
    useDistalToProximalView = user.isDistalToProximalView();
}

