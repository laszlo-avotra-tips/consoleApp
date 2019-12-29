#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

#include <queue>
#include <map>
#include <qmutex.h>
#include <QObject>

class FrameQueue : public QObject
{
    Q_OBJECT
public:
    explicit FrameQueue(QObject *parent = nullptr);

    void push(int frameId, void* buffer, int bufferSize);
    void pop();
    const std::pair<int, std::pair<void*,int>>& front();
    bool isEmpty() const;

signals:
    void queueChanged();

public slots:

private:
    std::queue<std::pair<int, std::pair<void*,int>>> m_frameQueue;
    std::map<int, std::pair<void*,int>> m_currentFrame;
    QMutex m_mutex;
    bool m_isEmpty;
};

#endif // FRAMEQUEUE_H
