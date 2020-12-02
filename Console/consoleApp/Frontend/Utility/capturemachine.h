/*
 * captureMachine.h
 *
 * The capture machine object handles all file and database
 * operations for image captures. It provides the model
 * and abstraction layer for dealing with the current
 * set of captures.
 *
 * Author: Chris White
 * Copyright (c) 2010-2018 Avinger, Inc.
 */

#pragma once

#include <QImage>
#include <QThread>
#include <QQueue>
#include <QMutex>

class captureMachine : public QThread
{
    Q_OBJECT

public:
    explicit captureMachine();

signals:
    void sendFileToKey( QString );
    void warning( QString );
    void error( QString );

public slots:
    void imageCapture( QImage decoratedImage, QImage sector, QString tagText, unsigned int timestamp, int pixelsPerMm, float zoom );
    void clipCapture( QImage sector, QString strClipNumber, unsigned int timestamp );

protected:
    void run();

private:
    // container for captures to put into a queue
    struct CaptureItem_t
    {
        QImage decoratedImage;
        QImage sectorImage;
        QString tagText;
        unsigned int timestamp;
        int   pixelsPerMm;
        float zoomFactor;
    };

    // container for clips to put into a queue
    struct ClipItem_t
    {
        QImage sectorImage;
        QString strClipNumber;
        unsigned int timestamp;
    };

    QQueue< CaptureItem_t > captureQ;
    QQueue< ClipItem_t > clipQ;

    // each type enqueues here to keep the thread alive as long as necessary
    QQueue< bool > runThreadQ;

    int currCaptureNumber;

    void processImageCapture(CaptureItem_t captureItem );
    void processLoopRecording( ClipItem_t loop );
    void addTimeStamp(QPainter& painter);
    void addFileName(QPainter& painter, const QString& fn);
    void addCatheterName(QPainter& painter);


    QMutex mutex;

};
