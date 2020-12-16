#ifndef OCTFRAMERECORDER_H
#define OCTFRAMERECORDER_H

#include <QObject>
#include "octFile.h"
extern "C" {
#include "Utility/ScreenCapture.hpp"
}

class ConcatenateVideo;

class OctFrameRecorder : public QObject
{
    Q_OBJECT

public:
    static OctFrameRecorder* instance();

    void onRecordSector(bool isRecording);
    bool recorderIsOn() const;
    void setRecorderIsOn(bool recorderIsOn);

    QString playlistFileName() const;
    void setPlaylistFileName(const QString &playlistFileName);

    bool start();
    bool stop();

    QString clipName() const;

    void setClipName(const QString &clipName);

signals:

public slots:
    void recordData(uint8_t *frame, int width, int height);

private:
    explicit OctFrameRecorder(QObject *parent = nullptr);
    void updateOutputFileName(int loopNumber);
    void updateClipList(int loopNumber);

    static OctFrameRecorder* m_instance;
    bool m_recorderIsOn{false};
    CapUtils::ScreenCapture* m_screenCapture{nullptr};
    QString m_playlistFileName;
    ConcatenateVideo* m_concatenateVideo{nullptr};
    const int m_width{1024};
    const int m_height{1024};

    QString m_clipName;
    QString m_timeStamp;

};

#endif // OCTFRAMERECORDER_H
