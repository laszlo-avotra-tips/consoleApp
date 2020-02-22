/*
 * dataConsumer.h
 *
 * Gets data from the shared memory pool.  The DAQ thread populates the memory,
 * this object uses it.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */

#ifndef DAQDATACONSUMER_H_
#define DAQDATACONSUMER_H_

#include <QQueue>
#include <QThread>
#include "advancedview.h"
#include "eventDataLog.h"
#include "livescene.h"
#include "octFile.h"
#include "Utility/directionTracker.h"
#include "videoencoder.h"

class DaqDataConsumer : public QThread
{
    Q_OBJECT

public:
    DaqDataConsumer( liveScene *s,
                     advancedView *adv,
                     EventDataLog *eLog );
    ~DaqDataConsumer();
    void stop( void ) { isRunning = false; }
    void disableFullCaseRecording( void ) { isAlwaysRecordFullCaseOn = false; }

signals:
    void updateAdvancedView( );
    void clipRecordingStopped();
    void directionOfRotation( directionTracker::Direction_T );
    void alwaysRecordingFullCase(bool);
    void videoClose();
    void rawVideoClose();
    void addFrame( char * );
    void sendVideoDuration( int );
    void updateBrightness( int );
    void updateContrast( int );

public slots:
    void handleTagEvent( QString );
    void recordBackgroundData( bool state );
    void setClipFile( QString clipFilename ) { clipFile = clipFilename; }
    void startClipRecording(void);
    void stopClipRecording(void);
    void updateCatheterView();
    void handleAutoAdjustBrightnessAndContrast( void );

protected:
    void run( void );

private:
    videoEncoder *clipEncoder;
    videoEncoder *caseEncoder;
    liveScene *sceneInThread;
    advancedView *advViewInThread;
    EventDataLog *eventLog;
    OCTFile::OctData_t m_octData;
    bool isRunning;
    bool isRecordFullCaseOn;
    bool isAlwaysRecordFullCaseOn;
    QSettings *systemSettings;
    QQueue<QString> addEventQ;

    unsigned long frameCount;
    int timeoutCounter;

    bool prevRecordClip;
    bool recordClip;

    QString strFullCaseNumber;

    QString clipFile;
    char *safeFrameBuffer;
    unsigned long clipFrameCount;
    unsigned long lastTimestamp;

    directionTracker dirTracker;
    directionTracker::Direction_T prevDirection;
    directionTracker::Direction_T currDirection;

    bool useDistalToProximalView;

    QTime processingTimer;
    QTime frameTimer;

    double millisecondsPerFrame;
    QMutex mutex;

    void setupEncoder( videoEncoder **cdc,
                       const QString VidFilename,
                       const int Width,
                       const int Height,
                       const bool IsRaw );

    // prevent access to default constructor, copy, and assign
    DaqDataConsumer();
    DaqDataConsumer( DaqDataConsumer const & );
    DaqDataConsumer & operator=( DaqDataConsumer const & );
};

#endif // DAQDATACONSUMER_H_
