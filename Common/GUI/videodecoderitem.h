/*
 * videodecoderitem.cpp
 *
 * Implements a QGraphicsPixmapItem capable of decoding and displaying
 * any video. Replaces the old Phonon framework (and DirectShow) based
 * object with much the same interface to clients.
 *
 * Author: Chris White
 * Copyright (c) 2013-2018 Avinger, Inc.
 */
#ifndef VIDEODECODERITEM_H
#define VIDEODECODERITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <qtimer>
extern "C" {
#include <avcodec.h>
#include <avformat.h>
#include <swscale.h>
}

class decoderState {

public:
    decoderState() : formatContext(NULL),
        codecContext(NULL), codec(NULL),
        frame(NULL), RGBFrame(NULL), rawFrameBuffer(NULL),
        videoStream(-1), millisecondsPerFrame(0),
        currentTime(0) {
    }
    ~decoderState() {

        // Close the codec
        if( codecContext ) {
           avcodec_close( codecContext );
        }
        if ( formatContext ) {
            av_close_input_file( formatContext );
        }
        if ( frame ) {
            av_free( frame );
        }
        if ( RGBFrame ) {
            av_free( RGBFrame );
        }
        if ( rawFrameBuffer ) {
            av_free( rawFrameBuffer );
        }
     }

    AVFormatContext *formatContext;
    AVCodecContext  *codecContext;
    AVCodec         *codec;
    AVFrame         *frame, *RGBFrame;
    uint8_t         *rawFrameBuffer;
    double           millisecondsPerFrame;
    int              videoStream;
    qint64            currentTime;
};

class decodeWorker : public QObject
{
    Q_OBJECT
public:
    explicit decodeWorker(decoderState *s) {
        frameImage = new QImage( s->codecContext->width, s->codecContext->height, QImage::Format_RGB888 );
        loopActive = false;
        running = false;
        state = s;
    }
    ~decodeWorker() {
        if (frameImage) {
            delete frameImage;
        }
    }

    bool isWorking() {
        return loopActive;
    }

public slots:
    void decodeFrame();
    void go() {
        running = true;
    }
    void stop() {
        running = false;
    }

signals:
    void newFrameAvailable(QImage *, qint64);
    void complete(void);
private:
    bool             running;
    bool             loopActive;
    QImage          *frameImage;
    decoderState     *state;
};

class videoDecoderItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit videoDecoderItem(QGraphicsItem *parent = 0);
    ~videoDecoderItem();
    enum State {
        IdleState,
        PlayingState,
        PausedState,
        ErrorState
    };
    qint64 totalTime();
    void play();
    void pause();
    bool load(QString);
    void clear();
    void setTickInterval(int);
    void seek(qint64);
    qint64 currentTime(void);
    enum State state() {
        return playbackState;
    }

signals:
    void hasVideoChanged(bool);
    void stateChanged(videoDecoderItem::State);
    void totalTimeChanged(qint64);
    void tick(qint64);
    void finished();

public slots:
    void updateFrame(QImage *, qint64);
    void tickTime(void);
    void videoEnd();
private:
    void decodeAndDisplayFirstFrame(void);
    decoderState    *videoState;
    enum State       playbackState;
    QTimer          *tickTimer;
    QTimer          *frameTimer;
    QThread         *decoderThread;
    decodeWorker    *decoder;
};

#endif // VIDEODECODERITEM_H
