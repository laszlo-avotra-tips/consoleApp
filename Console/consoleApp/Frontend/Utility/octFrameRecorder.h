#ifndef OCTFRAMERECORDER_H
#define OCTFRAMERECORDER_H

#include <QObject>

class OctFrameRecorder : public QObject
{
    Q_OBJECT

public:
    static OctFrameRecorder* instance();

signals:

private:
    explicit OctFrameRecorder(QObject *parent = nullptr);

    static OctFrameRecorder* m_instance;

};

#endif // OCTFRAMERECORDER_H
