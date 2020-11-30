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

    QString playlistThumbnail() const;
    void setPlaylistThumbnail(const QString &playlistThumbnail);

    int currentLoopNumber() const;

signals:

public slots:
    void recordData(uint8_t *frame);

private:
    explicit OctFrameRecorder(QObject *parent = nullptr);
    void updateOutputFileName(int loopNumber);
    void updateClipList(int loopNumber);

    static OctFrameRecorder* m_instance;
    int m_count{0};
    bool m_recorderIsOn{false};
    CapUtils::ScreenCapture* m_screenCapture{nullptr};
    QString m_playlistFileName;
    QString m_playlistThumbnail;
    int m_currentLoopNumber{0};
    ConcatenateVideo* m_concatenateVideo{nullptr};

};

#endif // OCTFRAMERECORDER_H
