
#include "theglobals.h"
#include "daqSettings.h"
#include "playbackmanager.h"
#include <QMutexLocker>

TheGlobals* TheGlobals::m_instance(nullptr);

TheGlobals *TheGlobals::instance()
{
    if(!m_instance){
        m_instance = new TheGlobals();
    }
    return m_instance;
}

TheGlobals::TheGlobals()
    :m_daqRawDataBufferCount(128),m_gFrameCounter(0),m_gDaqRawData_idx(0),
//      :m_daqRawDataBufferCount(192),m_gFrameCounter(0),m_gDaqRawData_idx(0),
      m_gDaqRawDataBuffersPosted(1),m_gDaqRawDataCompleted(0)
{
    allocateDaqRawDataBuffer();

    allocateFrameData();
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

        m_gFrameData.push_back(fd);
    }
}

const quint16 *TheGlobals::getDaqRawDataBuffer(size_t index) const
{
    const quint16* retVal(nullptr);
    if(index < m_daqRawData.size()){
        retVal = m_daqRawData[index];
    }
    return retVal;
}

const quint16 *TheGlobals::getDaqRawDataBuffer() const
{
    auto index = getGDaqRawData_idx();
    return getDaqRawDataBuffer(size_t(index));
}

void TheGlobals::freeDaqRawDataBuffer()
{
    for(auto it = m_daqRawData.begin(); it < m_daqRawData.end(); ++it){
        free(*it);
    }
    m_daqRawData.clear();
}

//5758976
void TheGlobals::allocateDaqRawDataBuffer(quint32 bufferSize)
{
    for(int i = 0; i < m_daqRawDataBufferCount; ++i){
        quint16* buffer = static_cast<quint16 *>(malloc( bufferSize ));
        m_daqRawData.push_back(buffer);
        PlaybackManager::instance()->addFrameBuffer(i,buffer, bufferSize); //TODO - optimize
    }
}

void TheGlobals::allocateDaqRawDataBuffer()
{
    const quint32 bytesPerBuffer(5758976);
    allocateDaqRawDataBuffer(bytesPerBuffer);
}

void TheGlobals::freeFrameData()
{
    for(auto it = m_gFrameData.begin(); it != m_gFrameData.end(); ++it){
        free(it->rawData);
        free(it->fftData);
        free(it->dispData);
        free(it->videoData);
    }
    m_gFrameData.clear();
}

OCTFile::FrameData_t *TheGlobals::getFrameDataPointer(int index) const
{
    OCTFile::FrameData_t * retVal(nullptr);
    if(size_t(index) < m_gFrameData.size()){
        auto& val = m_gFrameData[size_t(index)];
        retVal = const_cast<OCTFile::FrameData_t *>(&val);
    }
    return retVal;
}

OCTFile::FrameData_t *TheGlobals::getFrameDataPointer() const
{
    auto index = getGFrameCounter() % FRAME_BUFFER_SIZE;
    return getFrameDataPointer(index);
}

int TheGlobals::getGFrameCounter() const
{
    return m_gFrameCounter;
}

int TheGlobals::getPrevGFrameCounter() const
{
    return (m_gFrameCounter + FRAME_BUFFER_SIZE - 1) % FRAME_BUFFER_SIZE;
}

void TheGlobals::inrementGFrameCounter()
{
    QMutexLocker guard(&m_mutex);
    ++m_gFrameCounter;
}

int TheGlobals::getGDaqRawDataCompleted() const
{
    QMutexLocker guard(&m_mutex);
    return m_gDaqRawDataCompleted;
}

void TheGlobals::incrementGDaqRawDataCompleted()
{
    QMutexLocker guard(&m_mutex);
    ++m_gDaqRawDataCompleted;
}

int TheGlobals::getGDaqRawDataBuffersPosted() const
{
    return m_daqRawDataBufferCount;
}

int TheGlobals::getGDaqRawData_idx() const
{
    return m_gDaqRawData_idx;
}

int TheGlobals::getPrevGDaqRawData_idx() const
{
    return (m_gDaqRawData_idx + m_daqRawDataBufferCount - 1) % m_daqRawDataBufferCount;
}

void TheGlobals::updateGDaqRawData_idx()
{
    m_gDaqRawData_idx = m_gDaqRawDataCompleted % m_daqRawDataBufferCount;
}
