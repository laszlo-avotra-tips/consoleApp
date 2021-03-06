#pragma once

#include <string>
#include <memory>

#ifdef AVIDEORECORDER_EXPORTS
#define AVIDEORECORDER_API __declspec(dllexport)
#else
#define AVIDEORECORDER_API __declspec(dllimport)
#endif

namespace CapUtils {

    /*
    * Screen capture interface class that declares basic interfaces to perform screen capture recording
    * Derived class must implement all basic operations defined by this interface class.
    */
    class AVIDEORECORDER_API ScreenCaptureInterface {

    public:
        virtual ~ScreenCaptureInterface() = default;

        /*
        * @name Copy and move
        * 
        * No copying and moving allowed. These objects are always held by a smart pointer
        */
        ScreenCaptureInterface(const ScreenCaptureInterface&) = delete;
        ScreenCaptureInterface& operator=(const ScreenCaptureInterface&) = delete;

        ScreenCaptureInterface(ScreenCaptureInterface&&) = delete;
        ScreenCaptureInterface& operator=(ScreenCaptureInterface&&) = delete;

        /**
         * Start screen encode session.
         * This method blocks until screen capture is terminated by Stop command
         *
         * @param outDirPath
         *          Output file path where segmented files should be placed.
         *
         * @param playlistFileName
         *          Name of master playback file to play the whole capture session.
         *
         * @param fileNameLabel
         *          Label for loop file name to be displayed in the video.
         *
         * @param width
         *          Video output width resolution in pixels.
         *
         * @param height
         *          Video output height resolution in pixels.
         *
         * @param fps
         *          Frames Per Second. This is in the context of playback for output video
         *
         * @param crf
         *          Constant Rate Factor to adjust quality of output video. Default value is 23
         *          The value can be in the range of 1 - 51
         *
         * @param outputBitrateInMB
         *          Maximum output bitrate for the encoder to set quality for the output video.
         *          Default value is 0, meaning encoder chooses to use maximum bitrate that is not
         *          subjected to any peak values. The value value can be in the range of 8 - 50 MBps
         *
         * @param segmentDurationInSeconds
         *          Video segment duration. Default value is 5 seconds
         *
         * @return  True if capture session can be started after successfully initializing FFMPEG session.
         */
        virtual bool start(const char* outDirPath, const char* playlistFileName,
                           const char* fileNameLabel, int width, int height,
                           int fps, int crf, int outputBitrateInMB,
                           int segmentDurationInSeconds) = 0;
        /**
         * Encode input frame data to a segmented video transport stream through FFMPEG session
         *
         * @param data
         *     Screen pixel data captured from desktop.
         *
         * @param catheterName
         *     Name of catheter to be embedded into the video frame.
         *
         * @param catalogueName
         *     Catalogue name to be embedded into the video frame.
         *
         * @param activePassiveValue
         *     Active passive label indicating direction of rotation.
         *
         * @param timeLabel
         *     Current frame time to be embedded into the video.
         *
         * @return  True if the input is successfully encoded using FFMPEG session.
         */
        virtual bool encodeFrame(uint8_t* data, const char* catheterName, const char* catalogueName,
                                 const char* activePassiveValue, const char* timeLabel) = 0;

        /*
        * Stop capture module. Should stop any screen capture session that was already started
        */
        virtual void stop() = 0;

        /*
        * Set path for logo file. Contents from logo file will be embedded into image frame data
         *
         * @param logoFilePath
         *     Logo file path to fetch logo data
         *
        */
        virtual void setLogoPath(const char* logoFilePath) = 0;

    protected:
        ScreenCaptureInterface() = default;
    };

    /*
    * Screen capture class to grab screen region from desktop and stores them as a continuous
    * segmented transport streams. This is a top level interface class that can be used by
    * any app to carry out screen recording.
    */
    class ScreenCapture : public ScreenCaptureInterface {
    public:

        /**
         * ScreenCapture constructor. Class instance is instantiated by any App that intends to perform screen recording
         *
         * @param logPath
         *     Directory path to log file.
         *
         * @param logLevel
         *     Logger level. Default logger level is TRACE
         *
         */
        AVIDEORECORDER_API ScreenCapture(const char* logPath = nullptr, int logLevel = 0);

        virtual AVIDEORECORDER_API ~ScreenCapture();

        /*
        * @name Copy and move
        *
        * No copying and moving allowed. These objects are always held by a smart pointer
        */

        ScreenCapture(const ScreenCapture&) = delete;
        ScreenCapture& operator=(const ScreenCapture&) = delete;

        ScreenCapture(ScreenCapture&&) = delete;
        ScreenCapture& operator=(ScreenCapture&&) = delete;

        /*
        * Capture interface overrides
        */
        bool start(const char* outDirPath, const char* playlistFileName, const char* fileNameLabel,
                   int width = 1024, int height = 1024, int fps = 16, int crf = 23,
                   int outputBitrateInMB = 0, int segmentDurationInSeconds = 5) override;
        void stop() override;
        bool encodeFrame(uint8_t* data, const char* catheterName, const char* catalogueName, 
                         const char* activePassiveValue, const char* timeLabel) override;
        void setLogoPath(const char* logoFilePath) override;

    private:

        /*
        * Core screen capture session is implemented by PImpl technique to separate implementation details from interface
        */
        class Impl;
        std::shared_ptr<Impl> impl;
    };

    using ScreenCapture_p = std::shared_ptr<ScreenCapture>;
}
