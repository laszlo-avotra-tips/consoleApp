#ifndef OCTFRAMERECORDER_H
#define OCTFRAMERECORDER_H

#include <QObject>

class OctFrameRecorder : public QObject
{
    Q_OBJECT
public:
    explicit OctFrameRecorder(QObject *parent = nullptr);

signals:

};

#endif // OCTFRAMERECORDER_H
