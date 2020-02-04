
#include "theglobals.h"
#include "daqSettings.h"
#include "playbackmanager.h"
#include <QMutexLocker>
#include "logger.h"

TheGlobals* TheGlobals::m_instance(nullptr);

TheGlobals *TheGlobals::instance()
{
    if(!m_instance){
        m_instance = new TheGlobals();
    }
    return m_instance;
}

TheGlobals::TheGlobals()
    :m_rawDataBufferCount(64),m_frameIndex(0),m_rawDataIndex(0),
//    :m_rawDataBufferCount(192),m_frameIndex(0),m_rawDataIndex(0),
      m_rawDataIndexCompleted(0)
{
    allocateRawDataBuffer();

    allocateFrameData();
}

int TheGlobals::getFrameRenderingIndex() const
{
    return m_frameRenderingIndex;
}

void TheGlobals::allocateFrameData()
{
    DaqSettings &settings = DaqSettings::Instance();

    quint32 preTriggerSamples = settings.getPreDepth();

    // Select the number of post-trigger samples per record
    quint32 postTriggerSamples = settings.getRecordLength() - preTriggerSamples;

    for(int i = 0; i < FRAME_BUFFER_SIZE; ++i){

        OCTFile::FrameData_t fd;
        fd.rawData   = static_cast<unsigned short *>(malloc( (postTriggerSamples + preTriggerSamples) * sizeof( unsigned short ) ) ); // XXX: should be resampled size? (2048)
        fd.fftData   = static_cast<unsigned short *>(malloc( FFTDataSize * sizeof( unsigned short ) ) );
        fd.dispData  = static_cast<unsigned char *>(malloc( ( SectorHeight_px * SectorWidth_px ) * sizeof( unsigned char ) ) );
        fd.videoData = static_cast<unsigned char *>(malloc( ( SectorHeight_px * SectorWidth_px ) * sizeof( unsigned char ) ) );

        m_frameData.push_back(fd);
    }
}

const quint16 *TheGlobals::getRawDataBufferPointer(size_t index) const
{
    const quint16* retVal(nullptr);
    if(index < m_rawData.size()){
        retVal = m_rawData[index];
    }
    return retVal;
}

const quint16 *TheGlobals::getRawDataBufferPointer() const
{
    auto index = getRawDataIndex();
    return getRawDataBufferPointer(size_t(index));
}

void TheGlobals::freeRawDataBuffer()
{
    for(auto it = m_rawData.begin(); it < m_rawData.end(); ++it){
        free(*it);
    }
    m_rawData.clear();
}

//5758976
void TheGlobals::allocateRawDataBuffer(quint32 bufferSize)
{
    for(int i = 0; i < m_rawDataBufferCount; ++i){
        quint16* buffer = static_cast<quint16 *>(malloc( bufferSize ));
        m_rawData.push_back(buffer);
        PlaybackManager::instance()->addRawDataBuffer(i,buffer, bufferSize); //TODO - optimize
    }
}

void TheGlobals::allocateRawDataBuffer()
{
    const quint32 bytesPerBuffer(5758976);
    allocateRawDataBuffer(bytesPerBuffer);
}

bool TheGlobals::enqueueBuffer(int index)
{
    bool success(false);

    if(index < int(m_rawData.size())){
        m_rawDataQueue.push(index);
        success = true;
    }
    return true;
}

void TheGlobals::rawDataQueuePop(int index)
{
    QMutexLocker guard(&m_rawDataMutex);
    if(m_rawDataQueue.front() == index){
        m_rawDataQueue.pop();
    } else {
        LOG2(index, m_rawDataQueue.front())
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

bool TheGlobals::isRawDataQueue() const
{
    return !m_rawDataQueue.empty();
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

int TheGlobals::frontRawDataQueue() const
{
    if(isRawDataQueue()){
        return m_rawDataQueue.front();
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
        free(it->rawData);
        free(it->fftData);
        free(it->dispData);
        free(it->videoData);
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

void TheGlobals::inrementFrameIndex()
{
    QMutexLocker guard(&m_frameDataMutex);
    ++m_frameIndex;
}

int TheGlobals::getRawDataIndexCompleted() const
{
    return m_rawDataIndexCompleted;
}

void TheGlobals::incrementRawDataIndexCompleted()
{
    QMutexLocker guard(&m_rawDataMutex);
    ++m_rawDataIndexCompleted;
}

int TheGlobals::getRawDataBufferCount() const
{
    return m_rawDataBufferCount;
}

int TheGlobals::getRawDataIndex() const
{
    return m_rawDataIndex;
}

int TheGlobals::getPrevRawDataIndex() const
{
    return (m_rawDataIndex + m_rawDataBufferCount - 1) % m_rawDataBufferCount;
}

void TheGlobals::updateRawDataIndex()
{
    m_rawDataIndex = m_rawDataIndexCompleted % m_rawDataBufferCount;
}
