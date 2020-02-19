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

    int getRawDataIndex() const;
    int getPrevRawDataIndex() const;
    void updateRawDataIndex();

    int getRawDataBufferCount() const;

    int getRawDataIndexCompleted() const;
    void incrementRawDataIndexCompleted();

    int getFrameIndex() const;
    int getPrevFrameIndex() const;

    OCTFile::FrameData_t * getFrameDataPointer(int index) const;
    OCTFile::FrameData_t * getFrameDataPointer() const;
    void freeFrameData();
    void allocateFrameData();

    const quint16* getRawDataBufferPointer(size_t index) const;
    const quint16* getRawDataBufferPointer() const;
    void freeRawDataBuffer();
    void allocateRawDataBuffer(quint32 bufferSize);
    void allocateRawDataBuffer();
    bool enqueueBuffer(int index);
    void rawDataQueuePop(int index);
    void frameDataQueuePop(int index);
    bool isRawDataQueue() const;
    bool isFrameDataQueue() const;
    void pushFrameDataQueue(int index);
    void pushFrameRenderingQueue(int index);
    void popFrameRenderingQueue(int index);
    bool isFrameRenderingQueue() const;
    int  frontFrameRenderingQueue() const;
    int  frontRawDataQueue() const;
    int  frontFrameDataQueue() const;

    int getFrameRenderingIndex() const;

private:
    static TheGlobals* m_instance;
    TheGlobals();

private:
    // Create a block of memory for storing the data streaming
    // from the DAQ hardware. The array is indexed by m_gFrameCounter to point
    // to the current FrameDate_t to write to.  The memory is allocated all
    // at once to avoid calling on the OS to create and release memory during
    // run-time.
    std::vector<OCTFile::FrameData_t> m_frameData;
    std::vector<quint16*> m_rawData;
    QMutex m_rawDataMutex;
    QMutex m_frameDataMutex;
    const int m_rawDataBufferCount;
    int m_frameIndex;
    int m_rawDataIndex;
    int m_rawDataIndexCompleted;
    std::queue<int> m_rawDataQueue;
    std::queue<int> m_frameDataQueue;
    std::queue<int> m_frameRenderingQueue;
    std::queue<int> m_ImageQueue;
    int m_frameRenderingIndex;
};

#endif // THEGLOBALS_H
