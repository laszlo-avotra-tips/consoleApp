/*
 * videoencoder.h
 *
 * videoEncoder is a thread object which opens, configures, and
 * encodes video frames to a movie file using the ffmpeg encoding
 * library. The goal is to hide all of the complexity of movie
 * encoding from the clients, allowing them to simply pass in the
 * filename, start the thread and begin adding frames as raw
 * data buffers.
 */
#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

#include <QThread>
#include <QProgressDialog>

/*
 * Constants and video encode
 * parameters.
 */
#define DEFAULT_BITRATE_BPS (1000 * 10) // Video bitrate
#define DEFAULT_FPS     15
#define DEFAULT_BUFFERSIZE  int(8000 * 1024)
#define DEFAULT_GOP_INTERVAL 12

// Set for extra encode processing options.
#define HIGH_QUALITY_VIDEO 0

// ffmpeg libavcodec includes
//extern "C" {
//    #include "avformat.h"
//    #include "swscale.h"
//}

#define MAXFILENAMELEN 256

class videoEncoder : public QThread
{

Q_OBJECT
public:
    videoEncoder(char *filename, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double fps = DEFAULT_FPS);
    ~videoEncoder(void);
    void run();
    enum videoStatus_e
    {
        Starting,
        Ready,
        Failed
    };
    enum videoStatus_e initComplete(void) { return ready; }

    int getDuration( void ) { return duration_ms; }

    static void initVideoLibrary(void)
    {
        // Do this outside of the object, and only once.
//lcv        av_register_all();
    }

public slots:

    void addFrame(char *data);
    void close(void);

signals:
    void videoError(void);

private:
    bool init(void);
//    AVFrame *allocatePicture(int pixelFormat, int w, int h);
    bool initVideo(void);
//    AVStream *addVideoStream(int codecId);
//    void copyFrame(AVFrame *frame, char *frameData, AVCodecContext *codecCtx);
    void closeVideo(void);

//    AVFrame *picture, *tmpPicture;
    struct SwsContext *imgConvertCtx;

    unsigned char *outputBuffer;
    int frameCount;
    int duration_ms;
    int bufferSize;
    int width;
    int height;
    int finalWidth;
    int finalHeight;
    enum videoStatus_e ready;

    double framesPerSecond;
    int bitrate;
    int gopInterval;

    char filename[MAXFILENAMELEN];

//    AVOutputFormat *format;
//    AVFormatContext *outputCtx;
//    AVStream *videoStream;

    unsigned char *picBuf;

    double video_pts;
};

#endif // VIDEOENCODER_H
