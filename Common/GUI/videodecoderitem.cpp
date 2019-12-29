///*
// * videodecoderitem.cpp
// *
// * Implements a QGraphicsPixmapItem capable of decoding and displaying
// * any video. Replaces the old Phonon framework (and DirectShow) based
// * object with much the same interface to clients.
// *
// * Author: Chris White
// * Copyright (c) 2013-2018 Avinger, Inc.
// */
//#include "videodecoderitem.h"
//#include <qdebug>
//#include <qtime>
//#include <qthread>
//#include <QApplication>
//#include <QMutex>
//#include <QPainter>
//#include <QFile>
//#include "defaults.h"

//const qint64 MaxSeekOvershoot(1000); // One second error in the forward direction for seeking
//const qint64 MaxSeekUndershoot(500); // Half second error in the backward direction for seeking

//QTime StartStopTimer;

///*
// * constructor
// */
//videoDecoderItem::videoDecoderItem(QGraphicsItem *parent) :
//    QGraphicsPixmapItem(parent)
//{
//    videoState    = NULL;
//    decoder       = NULL;
//    decoderThread = NULL;
//    tickTimer     = NULL;
//    frameTimer    = NULL;
//    playbackState = IdleState;
//    setTransformationMode(Qt::SmoothTransformation);
//    av_register_all();
//}

///*
// * destructor
// */
//videoDecoderItem::~videoDecoderItem()
//{
//    if( frameTimer )
//    {
//        frameTimer->stop();
//        delete frameTimer;
//    }
//    if( decoder )
//    {
//        decoder->stop();
//        while (decoder->isWorking())
//        {
//            QApplication::processEvents();
//        }
//        delete decoder;
//    }
//    if( decoderThread )
//    {
//        decoderThread->exit();
//        decoderThread->wait();
//        delete decoderThread;
//    }
//    if( videoState )
//    {
//        // this call removes the pointer as well
//        avformat_close_input( &videoState->formatContext );
//    }
//    if( tickTimer )
//    {
//        delete tickTimer;
//    }
//}

///*
// * play
// */
//void videoDecoderItem::play( void )
//{
//    if( frameTimer && videoState )
//    {
//        qDebug() << "Starting frame timer at: " << (int)videoState->millisecondsPerFrame << "ms per update.";
//        decoder->go();
//        frameTimer->singleShot(0, decoder, SLOT(decodeFrame()));
//        playbackState = PlayingState;
//        emit stateChanged(PlayingState);
//    }
//    StartStopTimer.start();
//}

///*
// * pause
// */
//void videoDecoderItem::pause( void )
//{
//    if( frameTimer && videoState )
//    {
//        frameTimer->stop();
//        emit stateChanged(PausedState);
//    }
//    if( decoder )
//    {
//        decoder->stop();
//    }

//    // Make sure it's stopped
//    while (decoder->isWorking())
//    {
//        QApplication::processEvents();
//    }
//    playbackState = PausedState;

//}

///*
// * load
// */
//bool videoDecoderItem::load( QString filename )
//{

//    /*
//     * Are we still playing? STOP.
//     */
//    if(playbackState == PlayingState)
//    {
//        decoder->stop();
//        while (decoder->isWorking())
//        {
//            QApplication::processEvents(); // Allow it to definitely wind downf
//        }
//    }


//    /*
//     * Previous movie instance loaded?
//     */
//    if( videoState )
//    {
//        if( frameTimer )
//        {
//            frameTimer->stop();
//            delete frameTimer;
//            frameTimer = NULL;
//        }
//        if( decoderThread )
//        {
//            decoderThread->exit();
//            decoderThread->wait();
//            delete decoderThread;
//            decoderThread = NULL;
//        }

//        if( videoState )
//        {
//            delete videoState;
//            videoState = NULL;
//        }

//        if( decoder )
//        {
//            delete decoder;
//            decoder = NULL;
//        }
//    }

//    videoState = new decoderState();
//    playbackState = IdleState;

//    // If the file does not exist, it's an MP4 file from v2.x
//    if( !QFile::exists( filename ) )
//    {
//        filename.replace( LoopVideoExtension, LoopVideoExtension_v2 );
//    }

//    if( avformat_open_input( &videoState->formatContext, filename.toLatin1(), NULL, NULL) != 0 )
//    {
//        delete videoState;
//        videoState = NULL;
//        qDebug() << "videoDecoderItem::load failed to open movie file " << filename;
//        return false;
//    }

//    if( av_find_stream_info( videoState->formatContext ) < 0 )
//    {
//        delete videoState;
//        videoState = NULL;
//        qDebug() << "videoDecoderItem::load failed to find stream info.";
//        return false;
//    }

//    // Find the first video stream
//    for ( unsigned int streamCount = 0; streamCount < videoState->formatContext->nb_streams; streamCount++ )
//    {
//        if( videoState->formatContext->streams[streamCount]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
//        {
//            videoState->videoStream = streamCount;
//            break;
//        }
//    }

//    if( videoState->videoStream == -1 )
//    {
//        delete videoState;
//        videoState = NULL;
//        qDebug() << "videoDecoderItem::load failed to find video stream.";
//        return false;
//    }

//    videoState->codecContext = videoState->formatContext->streams[videoState->videoStream]->codec;
//    videoState->codec = avcodec_find_decoder( videoState->codecContext->codec_id );

//    if( videoState->codec == NULL )
//    {
//        delete videoState;
//        videoState = NULL;
//        qDebug() << "videoDecoderItem::could not find decoder.";
//        return false;
//    }

//    if( avcodec_open( videoState->codecContext, videoState->codec ) < 0 )
//    {
//        delete videoState;
//        videoState = NULL;
//        qDebug() << "videoDecoderItem::load failed to open codec.";
//        return false;
//    }

//    // Set up frame buffers
//    videoState->frame    = avcodec_alloc_frame();
//    videoState->RGBFrame = avcodec_alloc_frame();

//    videoState->rawFrameBuffer = (uint8_t *)av_malloc( avpicture_get_size(PIX_FMT_RGB24, videoState->codecContext->width,
//                                                                          videoState->codecContext->height) * sizeof(uint8_t) );
//    avpicture_fill( (AVPicture *)videoState->RGBFrame, videoState->rawFrameBuffer, PIX_FMT_RGB24,
//                    videoState->codecContext->width, videoState->codecContext->height );

//    AVStream *stream = videoState->formatContext->streams[videoState->videoStream];

//    // Get the frame rate from the stream, it has a more accurate view of the average rate.
//    videoState->millisecondsPerFrame = ((double)stream->avg_frame_rate.den / (double)stream->avg_frame_rate.num) * 1000.0;

//    decoderThread = new QThread(this);
//    decoder = new decodeWorker(videoState);

//    frameTimer = new QTimer(this);

//    connect( frameTimer, SIGNAL( timeout() ), decoder, SLOT( decodeFrame() ) );
//    connect( decoder, SIGNAL(newFrameAvailable(QImage *,qint64)), this, SLOT(updateFrame(QImage *,qint64)));
//    connect( decoder, SIGNAL( complete() ), this, SLOT( videoEnd() ) );
//    decoder->moveToThread(decoderThread);
//    decoderThread->start();

//    emit hasVideoChanged( true );
//    emit totalTimeChanged( totalTime() );
//    emit tick( currentTime() );
//    decodeAndDisplayFirstFrame();
//    return true;
//}


///*
// * clear
// */
//void videoDecoderItem::clear()
//{
//    pixmap().fill( Qt::black );
//}

///*
// * setTickInterval
// */
//void videoDecoderItem::setTickInterval( int interval )
//{
//    if( tickTimer )
//    {
//        delete tickTimer;
//        tickTimer = NULL;
//    }
//    tickTimer = new QTimer( this );
//    connect( tickTimer, SIGNAL( timeout() ), this, SLOT( tickTime() ) );
//    tickTimer->start( interval );
//}

///*
// * tickTime
// */
//void videoDecoderItem::tickTime( void )
//{
//    if( playbackState == PlayingState )
//    {
//        emit tick( currentTime() );
//    }
//}

///*
// * seek
// */
//void videoDecoderItem::seek( qint64 target )
//{
//    qint64 newTarget = target;
//    int flags = 0;

//    qDebug() << "Seeking to " << target;
//    static bool noReEntry = false;

//    if(noReEntry)
//    {
//        return; // Prevent multiple simultaneous calls by Qt event handler (like in the below wait loop).
//    }
//    if(playbackState == PlayingState)
//    {
//        decoder->stop();

//        while (decoder->isWorking())
//        {
//            noReEntry = true;
//            QApplication::processEvents(); // Allow it to definitely wind downf
//        }
//    }

//    if( target == 0 )
//    {
//        flags = AVSEEK_FLAG_ANY;
//    }
//    else
//    {
//        flags = AVSEEK_FLAG_FRAME;
//    }

//    if( target < videoState->currentTime )
//    {
//        flags |= AVSEEK_FLAG_BACKWARD;
//    }
//    if( avformat_seek_file(videoState->formatContext, videoState->videoStream, 0, target,
//                           totalTime(), flags ) < 0)
//    {
//        qDebug() << "Error while seeking.";
//    }
//    else
//    {
//        avcodec_flush_buffers(videoState->codecContext);
//    }

//    bool done = false;
//    AVPacket packet;

//    // Seek through to desired frame
//    while ( !done )
//    {
//        if( av_read_frame(videoState->formatContext, &packet) < 0 )
//        {
//            qDebug() << "Error reading frame while seeking.";
//            done = true;
//            break;
//        }

//        if( packet.stream_index == videoState->videoStream )
//        {
//            int frameFinished;
//            avcodec_decode_video2( videoState->codecContext,
//                                   videoState->frame, &frameFinished, &packet );

//            if( frameFinished )
//            {
//                int f = packet.pts;

//                // Zero in on the correct target if we missed. This may take few iterations
//                int error = f - target;
//                if( error > MaxSeekOvershoot )
//                { // Back up
//                    newTarget -= error;
//                    if(newTarget < 0)
//                    {
//                        newTarget = 0;
//                    }
//                    if( avformat_seek_file( videoState->formatContext, videoState->videoStream, 0, newTarget,
//                                            totalTime(), AVSEEK_FLAG_FRAME ) < 0 )
//                    {
//                        qDebug() << "Error while seeking.";
//                    }
//                    else
//                    {
//                        avcodec_flush_buffers(videoState->codecContext);
//                    }
//                    continue;
//                }
//                else if( error < MaxSeekUndershoot )
//                { // Stream forward
//                    continue;
//                }
//                else
//                { // In the goal zone.
//                    done = true;
//                    qDebug() << "Packet matches at f = " << f;
//                    videoState->currentTime = f;
//                    break;
//                }
//            }
//        }
//        av_free_packet( &packet );
//    }

//    qDebug() << "Sought to " << currentTime();
//    emit tick( currentTime() ); // Update the client, even if we aren't playing
//    if(playbackState == PlayingState)
//    {
//        decoder->go();
//        frameTimer->singleShot(0, decoder, SLOT(decodeFrame()));
//    }
//    noReEntry = false;
//}

///*
// * totalTime
// */
//qint64 videoDecoderItem::totalTime()
//{
//    if( videoState )
//    {
//        // Scale the duration by the timebase of the actual video stream
//        qint64 rescaled_time = av_rescale( videoState->formatContext->duration, videoState->formatContext->streams[videoState->videoStream]->time_base.den,
//                                           videoState->formatContext->streams[videoState->videoStream]->time_base.num );
//        int tmp =(qint64)( (double)rescaled_time / (double)AV_TIME_BASE);
//        return( tmp );
//    }
//    else
//    {
//        return 0;
//    }
//}

///*
// * currentTime
// */
//qint64 videoDecoderItem::currentTime()
//{
//    if( videoState )
//    {
//        return( videoState->currentTime );
//    }
//    else
//    {
//        return 0;
//    }
//}

///*
// * updateFrame
// */
//void videoDecoderItem::updateFrame( QImage *frame, qint64 /*timestamp*/ )
//{
//    static QTime elapsedFrameTime;

//    //  qDebug() << "Time between frame update: " << elapsedFrameTime.elapsed() << "ms";
//    QPixmap framePM = QPixmap::fromImage( *frame );
//    setPixmap( framePM );
//    update();
//    elapsedFrameTime.start();
//}


///*
// * decodeAndDisplayFirstFrame
// */
//void videoDecoderItem::decodeAndDisplayFirstFrame( void )
//{
//    struct SwsContext *img_convert_ctx;
//    AVPacket packet;
//    int frameFinished = 0;
//    bool repeat = true;

//    // Is this a packet from the video stream?
//    while ( repeat )
//    {
//        if( av_read_frame( videoState->formatContext, &packet ) >=0 )
//        {
//            if(packet.stream_index == videoState->videoStream)
//            {
//                // Decode video frame
//                avcodec_decode_video2( videoState->codecContext, videoState->frame, &frameFinished, &packet );

//                // Did we get a video frame?
//                if( frameFinished )
//                {
//                    img_convert_ctx = sws_getContext( videoState->codecContext->width,
//                                                      videoState->codecContext->height,
//                                                      videoState->codecContext->pix_fmt,
//                                                      videoState->codecContext->width,
//                                                      videoState->codecContext->height,
//                                                      PIX_FMT_RGB24,
//                                                      SWS_BICUBIC, NULL, NULL, NULL );

//                    // Convert the image from its native format to RGB
//                    sws_scale( img_convert_ctx, videoState->frame->data, videoState->frame->linesize, 0, videoState->codecContext->height,
//                               videoState->RGBFrame->data, ((AVPicture *)videoState->RGBFrame)->linesize );

//                    /*
//                     * Must release sws context after sws_getContext(). This is less significant here, because this is called only
//                     * at the first frame.
//                     */
//                    sws_freeContext( img_convert_ctx );

//                    // Send frame to display item
//                    QImage *frameImage = new QImage( videoState->codecContext->width, videoState->codecContext->height, QImage::Format_RGB888 );
//                    memcpy( frameImage->bits(), videoState->RGBFrame->data[0], videoState->codecContext->width * videoState->codecContext->height * 3 );
//                    updateFrame( frameImage, 0 );
//                    delete frameImage;
//                    repeat = false;
//                }
//            }

//            // Free the packet that was allocated by av_read_frame
//            av_free_packet(&packet);
//        }
//        else
//        {
//            qDebug() << "Error while reading video, or end of file reached.";
//            repeat = false;
//        }
//    }
//}

///*
// * videoEnd
// */
//void videoDecoderItem::videoEnd(void)
//{
//    pause();

//    emit totalTimeChanged(currentTime());
//    emit finished();
//}

///*
// * decodeFrame
// */
//void decodeWorker::decodeFrame(void)
//{
//    struct SwsContext *img_convert_ctx;
//    AVPacket packet;
//    int frameFinished = 0;
//    static QTime lastCalled;
//    QTime frameTime;
//    int   nextFrameTime = state->millisecondsPerFrame;

//    while ( running )
//    {
//        loopActive = true;

//        frameTime.start();
//        if( lastCalled.elapsed() < nextFrameTime )
//        {
//            QApplication::processEvents();
//            continue;
//        }

//        // Is this a packet from the video stream?
//        if( av_read_frame( state->formatContext, &packet ) >=0 )
//        {
//            if(packet.stream_index == state->videoStream)
//            {
//                // Decode video frame
//                avcodec_decode_video2( state->codecContext, state->frame, &frameFinished, &packet );

//                // Did we get a video frame?
//                if( frameFinished )
//                {
//                    img_convert_ctx = sws_getContext( state->codecContext->width,
//                                                      state->codecContext->height,
//                                                      state->codecContext->pix_fmt,
//                                                      state->codecContext->width,
//                                                      state->codecContext->height,
//                                                      PIX_FMT_RGB24,
//                                                      SWS_BICUBIC, NULL, NULL, NULL );

//                    // Convert the image from its native format to RGB
//                    sws_scale( img_convert_ctx, state->frame->data, state->frame->linesize, 0, state->codecContext->height,
//                               state->RGBFrame->data, ((AVPicture *)state->RGBFrame)->linesize );

//                    /*
//                     * Must release sws context after sws_getContext(). This fixes a significant memory leak, since this is called to
//                     * decode every video frame.
//                     */
//                    sws_freeContext( img_convert_ctx );

//                    // Send frame to display item
//                    memcpy( frameImage->bits(), state->RGBFrame->data[0], state->codecContext->width * state->codecContext->height * 3 );
//                    state->currentTime = packet.pts;

//                    emit newFrameAvailable( frameImage,  state->currentTime );
//                }
//            }
//            // Free the packet that was allocated by av_read_frame
//            av_free_packet( &packet );
//        }
//        else
//        {
//            emit complete();

//            // We're done.
//            running = false;
//        }
//        lastCalled.start();
//        nextFrameTime = state->millisecondsPerFrame - frameTime.elapsed(); // Adjust for frame decoding time
//    }
//    loopActive = false;
//}

