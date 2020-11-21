#ifndef OCTFRAMERECORDER_H
#define OCTFRAMERECORDER_H

#include <QObject>
#include "octFile.h"
extern "C" {
#include "Utility/ScreenCapture.hpp"
}

class OctFrameRecorder : public QObject
{
    Q_OBJECT

public:
    static OctFrameRecorder* instance();

    void onRecordSector(bool isRecording);
    bool recorderIsOn() const;
    void setRecorderIsOn(bool recorderIsOn);

    QString outDirPath() const;
    void setOutDirPath(const QString &outDirPath);

    QString playlistFileName() const;
    void setPlaylistFileName(const QString &playlistFileName);

    bool start();
    bool stop();

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
    QString m_outDirPath;
    QString m_playlistFileName;
    int m_currentLoopNumber{0};

};

#endif // OCTFRAMERECORDER_H
