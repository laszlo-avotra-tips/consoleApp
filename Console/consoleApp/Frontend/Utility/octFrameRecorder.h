#ifndef OCTFRAMERECORDER_H
#define OCTFRAMERECORDER_H

#include <QObject>
#include "octFile.h"

class OctFrameRecorder : public QObject
{
    Q_OBJECT

public:
    static OctFrameRecorder* instance();

signals:

public slots:
    void handleOctFrame(OCTFile::OctData_t *frame);

private:
    explicit OctFrameRecorder(QObject *parent = nullptr);

    static OctFrameRecorder* m_instance;
    int m_count{0};

};

#endif // OCTFRAMERECORDER_H
