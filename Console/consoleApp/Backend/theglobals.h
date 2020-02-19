#ifndef THEGLOBALS_H
#define THEGLOBALS_H

#include "octFile.h"
#include "defaults.h"
#include <QMutex>
#include <vector>
#include <queue>


class TheGlobals
{
public:
    static TheGlobals* instance();

    int getFrameIndex() const;
    int getPrevFrameIndex() const;

    OCTFile::FrameData_t * getFrameDataPointer(int index) const;
    OCTFile::FrameData_t * getFrameDataPointer() const;
    void freeFrameData();
    void allocateFrameData();

    void frameDataQueuePop(int index);
    bool isFrameDataQueue() const;
    void pushFrameDataQueue(int index);
    void pushFrameRenderingQueue(int index);
    void popFrameRenderingQueue(int index);
    bool isFrameRenderingQueue() const;
    int  frontFrameRenderingQueue() const;
    int  frontFrameDataQueue() const;

    int getFrameRenderingIndex() const;

private:
    static TheGlobals* m_instance;
    TheGlobals();

private:
    std::vector<OCTFile::FrameData_t> m_frameData;
    QMutex m_frameDataMutex;
    int m_frameIndex;
    std::queue<int> m_frameDataQueue;
    std::queue<int> m_frameRenderingQueue;
    int m_frameRenderingIndex;
};

#endif // THEGLOBALS_H
