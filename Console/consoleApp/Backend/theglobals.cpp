
#include "theglobals.h"
#include "daqSettings.h"
#include "playbackmanager.h"
#include <QMutexLocker>
#include "logger.h"

#include <memory>

TheGlobals* TheGlobals::m_instance(nullptr);

TheGlobals *TheGlobals::instance()
{
    if(!m_instance){
        m_instance = new TheGlobals();
    }
    return m_instance;
}

TheGlobals::TheGlobals()
    :
      m_frameIndex(0)
{
    allocateFrameData();
}

int TheGlobals::getFrameRenderingIndex() const
{
    return m_frameRenderingIndex;
}

void TheGlobals::allocateFrameData()
{
    DaqSettings &settings = DaqSettings::Instance();

    const size_t rawDataSize{settings.getRecordLength()}; //4096
    const size_t fftDataSize{FFTDataSize};
    const size_t dispDataSize{SectorHeight_px * SectorWidth_px};

    LOG3(rawDataSize, fftDataSize, dispDataSize); //8192, 4096, 1024, 1982464

    for(int i = 0; i < FRAME_BUFFER_SIZE; ++i){

        OCTFile::FrameData_t fd;

        fd.rawData   = new quint16 [rawDataSize];
        fd.fftData   = new quint16 [fftDataSize];
        fd.dispData  = new uchar [dispDataSize];
        fd.videoData = new uchar [dispDataSize];

        m_frameData.push_back(fd);
    }

}

void TheGlobals::frameDataQueuePop(int index)
{
    QMutexLocker guard(&m_frameDataMutex);
    if(index == m_frameDataQueue.front()){
        m_frameDataQueue.pop();
    } else {
        LOG2(index, m_frameDataQueue.front())
    }
}

bool TheGlobals::isFrameDataQueue() const
{
    return !m_frameDataQueue.empty();
}

void TheGlobals::pushFrameDataQueue(int index)
{
    QMutexLocker guard(&m_frameDataMutex);
    m_frameDataQueue.push(index);
}

void TheGlobals::pushFrameRenderingQueue(int index)
{
    QMutexLocker guard(&m_frameDataMutex);
    m_frameRenderingQueue.push(index);
}

void TheGlobals::popFrameRenderingQueue(int /*index*/)
{
    QMutexLocker guard(&m_frameDataMutex);
    m_frameRenderingIndex = m_frameRenderingQueue.front();
    m_frameRenderingQueue.pop();
}

bool TheGlobals::isFrameRenderingQueue() const
{
    return !m_frameRenderingQueue.empty();
}

int TheGlobals::frontFrameRenderingQueue() const
{
    if(isFrameRenderingQueue()){
        return m_frameRenderingQueue.front();
    }
    return -1;
}

int TheGlobals::frontFrameDataQueue() const
{
    if(isFrameDataQueue()){
        return m_frameDataQueue.front();
    }
    return -1;
}

void TheGlobals::freeFrameData()
{
    for(auto it = m_frameData.begin(); it != m_frameData.end(); ++it){
        delete [] it->rawData;
        delete [] it->fftData;
        delete [] it->dispData;
        delete [] it->videoData;
    }
    m_frameData.clear();
}

OCTFile::FrameData_t *TheGlobals::getFrameDataPointer(int index) const
{
    OCTFile::FrameData_t * retVal(nullptr);
    if(size_t(index) < m_frameData.size()){
        auto& val = m_frameData[size_t(index)];
        retVal = const_cast<OCTFile::FrameData_t *>(&val);
    }
    return retVal;
}

OCTFile::FrameData_t *TheGlobals::getFrameDataPointer() const
{
    auto index = getFrameIndex() % FRAME_BUFFER_SIZE;
    return getFrameDataPointer(index);
}

int TheGlobals::getFrameIndex() const
{
    return m_frameIndex;
}

int TheGlobals::getPrevFrameIndex() const
{
    return (m_frameIndex + FRAME_BUFFER_SIZE - 1) % FRAME_BUFFER_SIZE;
}
