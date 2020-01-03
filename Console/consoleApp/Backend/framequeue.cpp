#include "framequeue.h"
#include <QMutexLocker>
#include "logger.h"

FrameQueue::FrameQueue(QObject *parent) : QObject(parent), m_isEmpty(true)
{
}

void FrameQueue::push(int frameId, void *buffer, int bufferSize)
{
    std::pair< int, std::pair<void*,int>> thisBuffer(frameId,
                                                     std::pair<void*, int>(buffer,bufferSize));
    QMutexLocker guard(&m_mutex);
    LOG3(frameId, buffer, bufferSize)
    m_frameQueue.push(thisBuffer);
    emit queueChanged();
}

const std::pair<int, std::pair<void *, int> > &FrameQueue::front()
{
    QMutexLocker guard(&m_mutex);
    return m_frameQueue.front();
}

bool FrameQueue::isEmpty() const
{
    return m_frameQueue.empty();
}

void FrameQueue::pop()
{
    QMutexLocker guard(&m_mutex);
    m_frameQueue.pop();
    emit queueChanged();
}
