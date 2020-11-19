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

    bool recorderIsOn() const;
    void setRecorderIsOn(bool recorderIsOn);

signals:

public slots:
    void handleOctFrame(OCTFile::OctData_t *frame);

private:
    explicit OctFrameRecorder(QObject *parent = nullptr);

    static OctFrameRecorder* m_instance;
    int m_count{0};
    bool m_recorderIsOn{false};
    CapUtils::ScreenCapture* m_screenCapture{nullptr};

};

#endif // OCTFRAMERECORDER_H
