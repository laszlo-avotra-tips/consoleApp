#include <QFile>
#include <QDataStream>

#include "playbackmanager.h"
#include "logger.h"
#include "theglobals.h"


PlaybackManager* PlaybackManager::m_instance(nullptr);

PlaybackManager *PlaybackManager::instance()
{
    if(!m_instance){
        m_instance = new PlaybackManager();
    }
    return m_instance;
}

//void PlaybackManager::addRawDataBuffer(int index, void *fb, ulong size)
//{
////    LOG3(index,fb,size);
//    m_rawDataBufferContainer[index] = std::pair<void*,ulong>(fb,size);
//}

//void PlaybackManager::recordRawData(int index, int count)
//{
//    if(!isPlayback()){
//    m_dawDataBufferIndexQueue.push(index);
//        if(m_dawDataBufferIndexQueue.size() > m_rawDataBufferContainer.size()){
//            m_dawDataBufferIndexQueue.pop();
//        }
//    }
//    m_frameIndex = index;
//    m_countOfRawDataProcessed = count;
//}

//void PlaybackManager::retrieveRawData(int index, const void * /*buffer*/)
//{
//    LOG1(index);
//    if(isPlayback()){
//        const auto& it = m_rawDataBufferContainer.find(index);
//        buffer = it->second.first;
//        LOG2(index, buffer);
//    }
//    m_frameIndex = index;
//    ++m_countOfRawDataProcessed;
//}

//void PlaybackManager::saveBuffer(const QString &fn)
//{
//    if(!fn.isEmpty()){
//        LOG1(fn)
//        QFile outFile(fn);
//        if(!outFile.open(QIODevice::WriteOnly)){
//            QString errorMsg("error openeing file ");
//            errorMsg += fn;
//            LOG1(errorMsg)
//            return;
//        }

//        QDataStream qds(&outFile);

//        int i(0);
//        while(!m_dawDataBufferIndexQueue.empty()){
//            const auto& id = m_dawDataBufferIndexQueue.front();
//            m_dawDataBufferIndexQueue.pop();
//            const auto it = m_rawDataBufferContainer.find(id);
//            if(it != m_rawDataBufferContainer.end()){
//                void* pBuffer = it->second.first;
//                char* pData = static_cast<char*>(pBuffer);
//                auto size = it->second.second;
//                qds.writeRawData(pData, size);
//                if((i == 0) || (i == 194) || (i == 195)){
//                    LOG3(id,pBuffer,size)
//                }
//                ++i;
//            }
//        }
//    }
//}

//void PlaybackManager::loadBuffer(const QString &fn)
//{
//    if(!fn.isEmpty()){
////        m_isPlayback = true;
//        LOG2(m_isPlayback, fn)
//        QFile inFile(fn);
//        if(!inFile.open(QIODevice::ReadOnly)){
//            QString errorMsg("error openeing file ");
//            errorMsg += fn;
//            LOG1(errorMsg)
//            return;
//        }

//        QDataStream qds(&inFile);

//        int frameCount(0);
//        for (auto it = m_rawDataBufferContainer.begin(); it != m_rawDataBufferContainer.end(); ++it){
//            const auto& pointerSizePair = it->second;
//            char* buffer = static_cast<char*>(pointerSizePair.first);
//            auto readCount = qds.readRawData(buffer, pointerSizePair.second);
//            if(readCount != pointerSizePair.second){
//                QString errorMsg("file read error at index ");
//                errorMsg += QString::number(it->first);
//                LOG1(errorMsg)
//            }
//            ++frameCount;
////            LOG2(readCount,pointerSizePair.first);
//        }
//        LOG2(m_isPlayback, frameCount)
//        emit rawDataBuffersAvailable(frameCount);
//    }
//}

bool PlaybackManager::isPlayback() const
{
    return m_isPlayback;
}

PlaybackManager::PlaybackManager()
    : QObject(nullptr), m_isPlayback(false),m_playbackLoopSleep(1),m_isSingleStep(false)
{
}

bool PlaybackManager::isSingleStep()
{
    bool retVal = m_isSingleStep;
    m_isSingleStep = false;
    return retVal;
}

void PlaybackManager::setCount(int count, int index)
{
    auto it = m_countContainer.begin();

    if(it == m_countContainer.end()){
        m_countContainer.push_back(count);
        m_countContainer.push_back(index);
        emit countChanged(count, index);
    } else if(count != *it){
        m_countContainer.clear();
        m_countContainer.push_back(count);
        m_countContainer.push_back(index);
        emit countChanged(count, index);
    }
}

void PlaybackManager::singleStep()
{
    m_isSingleStep = true;
    LOG1(m_isSingleStep)
}

unsigned long PlaybackManager::playbackLoopSleep() const
{
    return m_playbackLoopSleep;
}

//bool PlaybackManager::isInputQueue() const
//{
//    return TheGlobals::instance()->isRawDataQueue();
//}

//bool PlaybackManager::EnqueueBuffer(int index)
//{
//    return TheGlobals::instance()->enqueueBuffer(index);
//}

//bool PlaybackManager::findInputBuffer(int index, void *&dataBuffer)
//{
//    bool success = false;
//    auto it = m_rawDataBufferContainer.find(index);

//    if(it != m_rawDataBufferContainer.end()){
//        dataBuffer = it->second.first;
//        success = true;
//    }

//    return success;
//}

bool PlaybackManager::findDisplayBuffer(int index, OCTFile::FrameData_t *&frameData)
{
    frameData = TheGlobals::instance()->getFrameDataPointer(index);

    return frameData != nullptr;
}

//void PlaybackManager::inputProcessingDone(int index)
//{
//    TheGlobals::instance()->rawDataQueuePop(index);
//    TheGlobals::instance()->pushFrameDataQueue(index);
//}

//bool PlaybackManager::isFrameQueue() const
//{
//    return TheGlobals::instance()->isFrameDataQueue();
//}

void PlaybackManager::frameReady(int index)
{
    TheGlobals::instance()->pushFrameRenderingQueue(index);
}

void PlaybackManager::startPlayback()
{
    m_isPlayback = true;
    LOG1(m_isPlayback)
}

void PlaybackManager::stopPlayback()
{
    m_isPlayback = false;
    LOG1(m_isPlayback)
}

void PlaybackManager::setPlaybackSpeed(int speed)
{
    unsigned long maxSleep(1001);
    m_playbackLoopSleep = maxSleep - quint32(speed);
}

//int PlaybackManager::frameIndex() const
//{
//    return m_frameIndex;
//}

//int PlaybackManager::countOfRawDataBuffersProcessed() const
//{
//    return m_countOfRawDataProcessed;
//}

//int PlaybackManager::queueSize() const
//{
//    return int(m_dawDataBufferIndexQueue.size());
//}
