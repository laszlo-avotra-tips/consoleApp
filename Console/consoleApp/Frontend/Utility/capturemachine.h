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
    void processLoopRecording( ClipItem_t clipItem );
    void addTimeStamp(QPainter& painter, bool isClip = false);
    void addFileName(QPainter& painter, const QString& fn, bool isClip = false);
    void addCatheterName(QPainter& painter, bool isClip = false);
    void addLogo(QPainter& painter, bool isClip = false);
    QString generateImageName();
    void saveImage(const QImage &decoratedImage, const QString& imageName);
    void saveThumbnail(const QImage &decoratedImage, const QString& imageName);
    void addCaptureToTheModel(const CaptureItem_t &captureItem, const QString& imageName);
    QString generateClipFileName(const ClipItem_t& clipItem);


    QMutex mutex;

};
