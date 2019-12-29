/*
 * videoencoder.cpp
 *
 * videoEncoder is a thread object which opens, configures, and
 * encodes video frames to a movie file using the ffmpeg encoding
 * library. The goal is to hide all of the complexity of movie
 * encoding from the clients, allowing them to simply pass in the
 * filename, start the thread and begin adding frames as raw
 * data buffers.
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 */
#include "videoencoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include <QApplication>
#include <QSettings>
#include "buildflags.h"
#include "defaults.h"
#include "logger.h"

#if ENABLE_VIDEO_CRF_QUALITY_TESTING
extern QString gCrfTestValue;
#endif

const int UsecPerMsec = 1000;

/*
 * Constructor
 */
videoEncoder::videoEncoder(char *vidFilename, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double fps)
{
    imgConvertCtx = NULL;
    picture       = NULL;
    tmpPicture    = NULL;
    imgConvertCtx = NULL;
    outputBuffer  = NULL;
    format        = NULL;
    videoStream   = NULL;

    frameCount      = 0;
    framesPerSecond = fps;
    bitrate         = DEFAULT_BITRATE_BPS;
    bufferSize      = DEFAULT_BUFFERSIZE;
    gopInterval     = DEFAULT_GOP_INTERVAL;

    width       = srcWidth;
    height      = srcHeight;
    finalWidth  = dstWidth;
    finalHeight = dstHeight;
    ready = Starting;
    strncpy_s(filename, vidFilename, MAXFILENAMELEN);
}

/*
 * Destructor
 */
videoEncoder::~videoEncoder(void)
{
}

/*
 * run()
 *
 * Just run the event loop so we can process messages
 */
void videoEncoder::run(void)
{
    if( init() )
    {
        ready = Ready;
    }
    else
    {
        ready = Failed;
        return;
    }
    exec();
}

/*
 * allocatePicture()
 *
 * Allocate and configure the avpicture object for the library
 * This sets up a buffer with the format and size we want for
 * encoding.
 */
AVFrame *videoEncoder::allocatePicture(int pixelFormat, int width, int height)
{
    AVFrame *p;
    int size;

    p = avcodec_alloc_frame();
    if(!p)
    {
        return(NULL);
    }

    size = avpicture_get_size((PixelFormat)pixelFormat, width, height);
    picBuf = (unsigned char *)av_malloc(size);
    if(!picBuf)
    {
        av_free(p);
        return(NULL);
    }
    avpicture_fill((AVPicture *)p, picBuf, (PixelFormat)pixelFormat, width, height);
    return(p);
}

/*
 * initVideo()
 *
 * Initialize the video subsystem. Create the codec and
 * buffers necessary for encoding.
 */
bool videoEncoder::initVideo(void)
{
    AVCodecContext *codecCtx;
    AVCodec        *codec;
    AVDictionary    *opts = NULL;
    int             status;

    codecCtx = videoStream->codec;

    codec = avcodec_find_encoder(CODEC_ID_H264);
    if(!codec)
    {
        qDebug() << "Failed to find encoder for preferred format.";
        return(false);
    }

    /*
     * Load the system video quality setting.
     */
    QSettings settings( SystemSettingsFile, QSettings::IniFormat );
    int crfSetting = settings.value( VideoQualitySetting, VideoQualityLevel_crf.defaultValue ).toInt();

    if( ( crfSetting < VideoQualityLevel_crf.minValue ) || ( crfSetting > VideoQualityLevel_crf.maxValue ) )
    {
        crfSetting = VideoQualityLevel_crf.defaultValue;
    }

    /*
     * Constant Rate Factor (CRF)
     * range: 0-51
     *      0: lossless; largest file sizes
     *     23: default
     *     51: worst possible; smallest file sizes
     * Recommended: 15-28
     *
     * References:
     *   http://ffmpeg.org/trac/ffmpeg/wiki/x264EncodingGuide
     *   http://www.twm-kd.com/linux/encoding-videos-with-ffmpeg-and-h-264/
     */
#if ENABLE_VIDEO_CRF_QUALITY_TESTING
    // If enabled, the value in system.ini is completely ignored!
    av_dict_set(&opts, "crf", gCrfTestValue.toAscii().constData(), 0);
    qDebug() << "====> crf = " << gCrfTestValue;
    LOG( DEBUG, QString( "video encoder crf (test value) = %1" ).arg( gCrfTestValue ) );
#else
    av_dict_set( &opts, "crf", QString( "%1" ).arg( crfSetting ).toAscii().constData(), 0 );
    LOG( DEBUG, QString( "video encoder crf = %1" ).arg( crfSetting ) );
#endif

    if( ( status = avcodec_open2(codecCtx, codec, &opts ) ) < 0 )
    {
        qDebug() << "Failed to open codec context, errno: " << status;
        return(false);
    }

    outputBuffer = NULL;
    if(!(outputCtx->oformat->flags & AVFMT_RAWPICTURE))
    {

        // Allocate output buffer
        outputBuffer = (unsigned char *)av_malloc(bufferSize);
    }

    picture = allocatePicture(codecCtx->pix_fmt, codecCtx->width, codecCtx->height);
    if(!picture)
    {
        qDebug() << "Failed to allocate picture buffer.";
        return(false);
    }

    /*
     * Not YUV? Then we need to convert using this intermediate picture buffer
     */
    tmpPicture = NULL;
    if(codecCtx->pix_fmt != PIX_FMT_GRAY8)
    {
        tmpPicture = allocatePicture(PIX_FMT_GRAY8, width, height);
        if(!tmpPicture)
        {
            qDebug() << "Failed to allocate colorspace conversion buffer.";
            return(false);
        }
    }
    return(true);
}

/*
 * addVideoStream()
 *
 * Allocate a new video stream for encoding. This is
 * where all the codec parameters get set (look at the ffmpeg
 * docs for details, most of these map to command-line
 * options of one sort or another). The defaults here
 * were taken from the ffmpeg faq more or less for
 * "good quality" h264.
 */
AVStream *videoEncoder::addVideoStream(int codecId)
{
    AVStream *stream;
    AVCodecContext *codecCtx;

    stream = av_new_stream(outputCtx, 0);
    if(!stream)
    {
        qDebug() << "Failed to create new video stream.";
        return NULL;
    }

    codecCtx = stream->codec;
    codecCtx->codec_id = (CodecID)codecId;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    codecCtx->thread_count = 4;
    // Default test parameters, dial these in based
    // on quality/performance needs XXX
    codecCtx->width    = finalWidth;
    codecCtx->height   = finalHeight; // height;
    
    /*
     * to get more accurate timing in the videos, we convert FPS back to RPM 
     * and let the codec compute FPS with its internal datatype (AVRational).
     *
     * time_base is calculated by time_base.num / time_base.den; The result
     * should be in units of 1/fps = (sec/frame) or just seconds.
     */
    codecCtx->time_base.num = 60;
    codecCtx->time_base.den = (int)(( framesPerSecond * 60.0 ) + 0.5); // Add 0.5 to rounding (double) to (int).
    codecCtx->gop_size = framesPerSecond * 10;
    codecCtx->pix_fmt = PIX_FMT_YUV420P; // pixel format, this requires translation from RGB->YUV colorspace

    codecCtx->coder_type = 1;  // coder = 1

    codecCtx->me_cmp = 1; //
    codecCtx->me_range = 16; //
    codecCtx->qmin = 10;//
    codecCtx->qmax = 51;//
    codecCtx->qcompress = 0.6f; //
    codecCtx->max_qdiff = 4;//

    // some formats want stream headers to be separate
    // strcmp returns 0 when strings are equal.  Thanks, C.
    if( !strcmp( outputCtx->oformat->extensions, "mp4" ) ||
        !strcmp( outputCtx->oformat->extensions, "mov" ) ||
        !strcmp( outputCtx->oformat->extensions, "3gp" ) ||
        !strcmp( outputCtx->oformat->extensions, "mkv" ) )
    {
        codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    return(stream);
}

/*
 * init()
 *
 * Set up av library for encoding, allocate all necessary
 * codecs and buffers.
 */
bool videoEncoder::init(void)
{
    format = av_guess_format(NULL, filename, NULL);
    if(!format)
    {
        qDebug() << "Couldn't determine format from filename.";
        return(false);
    }

    outputCtx = avformat_alloc_context();
    if(!outputCtx)
    {
        qDebug() << "Failed to allocate format context.";
        return(false);
    }

    outputCtx->oformat = format;
#ifdef Q_WS_WIN
    _snprintf_s(outputCtx->filename, sizeof(outputCtx->filename), "%s", filename);
#else
    snprintf(outputCtx->filename, sizeof(outputCtx->filename), "%s", filename);
#endif
    if(format->video_codec != CODEC_ID_NONE)
    {
        videoStream = addVideoStream(format->video_codec);
        qDebug() << "videoStream added:" << videoStream;
    }

    if(videoStream)
    {
        if(!initVideo())
        {
            qDebug() << "Video encoder init failed.";
            return(false);
        }
    }

    /*
     * Open the output file
     */
    if(!(format->flags & AVFMT_NOFILE))
    {
        if(avio_open(&outputCtx->pb, filename, AVIO_FLAG_WRITE) < 0)
        {
            qDebug() << "Failed to open output file " << filename;
            return(false);
        }
    }
    avformat_write_header(outputCtx, NULL);
    return(true);
}

/*
 * copyFrame()
 *
 * Simple function to memcpy frame data to an AVFrame, was
 * more complicated earlier.
 */
void videoEncoder::copyFrame(AVFrame *frame, char *frameData, AVCodecContext *)
{
    memcpy(frame->data[0], frameData, width * height);
}

#ifdef Q_WS_WIN
/*
 * hacked_avcodec_encode_video()
 *
 * Here be dragons. This wrapper around avcodec_encode_video()
 * ensures that the stack is aligned in a way that makes
 * ffmpeg happy. This is horrible and only necessary when building
 * on windows. Newer versions of ffmpeg may not need this. Otherwise
 * really don't touch.
 */
__declspec(naked)
        static int hacked_avcodec_encode_video(
                AVCodecContext* c,
                uint8_t* video_outbuf,
                int video_outbuf_size,
                const AVFrame* picture)
{
    _asm {
        push ebp
                mov ebp,esp
                sub esp,__LOCAL_SIZE /*
                              * not really needed,
                              * but safer against errors
                              * when changing this function
                              */
                /* adjust stack to 16 byte boundary */
                and esp,~0x0f
            }
    avcodec_encode_video(c, video_outbuf,
                         video_outbuf_size, picture);
    _asm {
        mov esp,ebp
                pop ebp
                ret
            }
}
#endif

/*
 * addFrame()
 *
 * addFrame is the function of most interest to clients.
 * Given a buffer of image data (in whatever format and size
 * the video was set up for), it adds it to the video as
 * an encoded frame. Note this does no input checking and
 * assumes the client is sending a buffer of the right size
 * and format.
 */
void videoEncoder::addFrame(char *frameData)
{
    int outSize = -2;
    int retval = 0;
    AVCodecContext *codecCtx;

    if( videoStream == NULL )
    {
        /*
         * if the videoStream got stepped on, log the issue and return
         * without freaking out.
         */
        LOG( DEBUG, "videoEncoder: Error adding frame..." );
        qDebug() << "videoEncoder: Error adding frame...";
        return;
    }

    codecCtx = videoStream->codec;

    /*
     * If the codec is not grayscale, we need to convert
     * from grayscale format to whatever the codec
     * was set up for.
     */
    if(codecCtx->pix_fmt != PIX_FMT_GRAY8)
    {

        if(imgConvertCtx == NULL)
        {
            imgConvertCtx = sws_getContext(width, height,
                                           PIX_FMT_GRAY8, codecCtx->width, codecCtx->height,
                                           codecCtx->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);

            if(imgConvertCtx == NULL)
            {
                qDebug() << "Failed to get image conversion context.";
                QApplication::exit(-1);
                quit();
            }
        }
        copyFrame(tmpPicture, frameData, codecCtx);

        /*
         * Do the converstion.
         */
        sws_scale(imgConvertCtx, tmpPicture->data, tmpPicture->linesize,
                  0, height, picture->data, picture->linesize);
    }
    else
    {

        /*
         * Otherwise, just copy it on in.
         */
        copyFrame(picture, frameData, codecCtx);
    }

    /*
     * Do the encode
     */
    if(outputCtx->oformat->flags & AVFMT_RAWPICTURE)
    {

        /*
         * Raw video? Do nothing special, just write the frame.
         */
        AVPacket packet;
        av_init_packet(&packet);

        packet.flags |= AV_PKT_FLAG_KEY;
        packet.stream_index = videoStream->index;
        packet.data = (unsigned char *)picture;
        packet.size = sizeof(AVPicture);

        retval = av_write_frame(outputCtx, &packet);
    }
    else
    {
        /*
         * Encode the frame
         */
        outSize = hacked_avcodec_encode_video(codecCtx, outputBuffer, bufferSize, picture);

        /*
         * if zero size, then the image was buffered in the encoder, we'll get to it later.
         */
        if(outSize > 0)
        {
            AVPacket packet;

            av_init_packet(&packet);

            if(codecCtx->coded_frame->pts != AV_NOPTS_VALUE)
            {
                packet.pts = av_rescale_q(codecCtx->coded_frame->pts, codecCtx->time_base, videoStream->time_base);
            }
            if(codecCtx->coded_frame->key_frame)
            {
                packet.flags |= AV_PKT_FLAG_KEY;
            }
            packet.stream_index = videoStream->index;
            packet.data = outputBuffer;
            packet.size = outSize;

            /*
             * write the compressed frame to the video file
             */
            retval = av_write_frame(outputCtx, &packet);
        }
        else
        {
            retval = 0;
        }
    }
    if(retval != 0)
    {
        qDebug() << "Error writing frame: " << retval;
        emit videoError();
        return;
    }
    frameCount++;
}

/*
 * closeVideo()
 *
 * We're done with an encode, close down the codec and free the
 * buffers.
 */
void videoEncoder::closeVideo(void)
{
    avcodec_close(videoStream->codec);
    av_free(picture->data[0]);
    av_free(picture);
    if(tmpPicture)
    {
        av_free(tmpPicture->data[0]);
        av_free(tmpPicture);
    }
    av_free(outputBuffer);

    /*
     * Must release sws context after calling sws_getContext(). Without, a memory leak is created
     * with each loop recorded.
     */
    sws_freeContext( imgConvertCtx );
}

/*
 * close()
 *
 * Finish an encode. Deal with any remaining buffers in the video
 * stream, then, write out the video trailer if necessary and close
 * the file.
 */
void videoEncoder::close(void)
{
    AVCodecContext *codecCtx;
    int             outSize(-2);

    qDebug() << "Closing video...";
    codecCtx = videoStream->codec;

    /*
     * Drain any remaining frames
     */
#ifdef Q_WS_WIN
        outSize = hacked_avcodec_encode_video(codecCtx, outputBuffer, bufferSize, NULL);
#else
        outSize = avcodec_encode_video(codecCtx, outputBuffer, bufferSize, picture);
#endif
    while (outSize)
    {

        // if zero size, then the image was buffered in the encoder
        if(outSize > 0)
        {
            AVPacket packet;

            av_init_packet(&packet);

            if(codecCtx->coded_frame->pts != AV_NOPTS_VALUE)
            {
                packet.pts = av_rescale_q(codecCtx->coded_frame->pts, codecCtx->time_base, videoStream->time_base);
            }
            if(codecCtx->coded_frame->key_frame)
            {
                packet.flags |= AV_PKT_FLAG_KEY;
            }
            packet.stream_index = videoStream->index;
            packet.data = outputBuffer;
            packet.size = outSize;

            /*
             * write the compressed frame to the video file
             */
            av_write_frame(outputCtx, &packet);
        }
        else if(outSize < 0)
        {
            qDebug() << "Error draining frame.";
            outSize = 0;
        }
#ifdef Q_WS_WIN
        outSize = hacked_avcodec_encode_video(codecCtx, outputBuffer, bufferSize, NULL);
#else
        outSize = avcodec_encode_video(codecCtx, outputBuffer, bufferSize, picture);
#endif
    }

    closeVideo();
    qDebug() << "Writing trailer...";
    av_write_trailer(outputCtx);

    qDebug() << "Freeing buffers...";

    // Free streams
    for( unsigned int i = 0; i < outputCtx->nb_streams; i++ )
    {
        av_freep(&outputCtx->streams[i]->codec);
        av_freep(&outputCtx->streams[i]);
    }

    if(!(format->flags & AVFMT_NOFILE))
    {
        // Close the output file
        avio_close(outputCtx->pb);
    }

    // Free the stream
    av_free(outputCtx);

    /*
     * The duration field is only filled in when loading a movie; re-open
     * to get the duration of the entire clip before closing the encoder.
     */
    duration_ms = 0;
    outputCtx   = avformat_alloc_context();
    avformat_open_input( &outputCtx, filename, NULL, NULL );

    /*
     * If the file contains no data (header only), then outputCtx will be null
     * and all data is invalid.  If it did load data, use it to get the duration
     * and then set it free.
     */
    if( outputCtx != NULL )
    {
        duration_ms = ( outputCtx->duration / UsecPerMsec );

        avformat_close_input( &outputCtx );
        av_free( outputCtx );
    }
    qDebug() << "Closing video thread.";

    // Kill this thread.  XXX: not the cleanest way to exit
    terminate();
}
