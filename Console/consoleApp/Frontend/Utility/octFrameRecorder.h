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
    const int m_width{2160};
    const int m_height{2159};

};

#endif // OCTFRAMERECORDER_H
