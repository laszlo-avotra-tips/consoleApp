#ifndef CONCATENATEVIDEO_H
#define CONCATENATEVIDEO_H

#include <QObject>

class ConcatenateVideo : public QObject
{
    Q_OBJECT
public:
    static ConcatenateVideo* instance();

private:
    explicit ConcatenateVideo(QObject *parent = nullptr);

signals:

private:
    static ConcatenateVideo* m_instance;

};

#endif // CONCATENATEVIDEO_H
