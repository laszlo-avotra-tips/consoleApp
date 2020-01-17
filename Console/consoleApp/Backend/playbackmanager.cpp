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
        LOG1(m_instance)
    }
    return m_instance;
}

void PlaybackManager::addFrameBuffer(int index, void *fb, ulong size)
{
//    LOG3(index,fb,size);
    m_frameBufferContainer[index] = std::pair<void*,ulong>(fb,size);
}

void PlaybackManager::recordFrameData(int index, int count)
{
    if(!isPlayback()){
    m_validFrameIndexCache.push(index);
        if(m_validFrameIndexCache.size() > m_frameBufferContainer.size()){
            m_validFrameIndexCache.pop();
        }
    }
    m_frameIndex = index;
    m_countDaqRawDataCompleted = count;
}

void PlaybackManager::retrieveFrameData(int index, const void *buffer)
{
//    LOG1(index);
    if(isPlayback()){
        const auto& it = m_frameBufferContainer.find(index);
        buffer = it->second.first;
//        LOG2(index, buffer);
    }
    m_frameIndex = index;
    ++m_countDaqRawDataCompleted;
}

void PlaybackManager::saveBuffer(const QString &fn)
{
    if(!fn.isEmpty()){
        LOG1(fn)
        QFile outFile(fn);
        if(!outFile.open(QIODevice::WriteOnly)){
            QString errorMsg("error openeing file ");
            errorMsg += fn;
            LOG1(errorMsg)
            return;
        }

        QDataStream qds(&outFile);

        int i(0);
        while(!m_validFrameIndexCache.empty()){
            const auto& id = m_validFrameIndexCache.front();
            m_validFrameIndexCache.pop();
            const auto it = m_frameBufferContainer.find(id);
            if(it != m_frameBufferContainer.end()){
                void* pBuffer = it->second.first;
                char* pData = static_cast<char*>(pBuffer);
                auto size = it->second.second;
                qds.writeRawData(pData, size);
                if((i == 0) || (i == 194) || (i == 195)){
                    LOG3(id,pBuffer,size)
                }
                ++i;
            }
        }
    }
}

void PlaybackManager::loadBuffer(const QString &fn)
{
    if(!fn.isEmpty()){
//        m_isPlayback = true;
        LOG2(m_isPlayback, fn)
        QFile inFile(fn);
        if(!inFile.open(QIODevice::ReadOnly)){
            QString errorMsg("error openeing file ");
            errorMsg += fn;
            LOG1(errorMsg)
            return;
        }

        QDataStream qds(&inFile);

        int frameCount(0);
        for (auto it = m_frameBufferContainer.begin(); it != m_frameBufferContainer.end(); ++it){
            const auto& pointerSizePair = it->second;
            char* buffer = static_cast<char*>(pointerSizePair.first);
            auto readCount = qds.readRawData(buffer, pointerSizePair.second);
            if(readCount != pointerSizePair.second){
                QString errorMsg("file read error at index ");
                errorMsg += QString::number(it->first);
                LOG1(errorMsg)
            }
            ++frameCount;
//            LOG2(readCount,pointerSizePair.first);
        }
//#if QT_NO_DEBUG
//        const auto frameIt = m_frameBufferContainer.begin();
//        m_frameQueue.push(frameIt->first, frameIt->second.first, frameIt->second.second);
//#endif
        LOG2(m_isPlayback, frameCount)
        emit framesAvailable(frameCount);
    }
}

bool PlaybackManager::isPlayback() const
{
    return m_isPlayback;
}

bool PlaybackManager::thereAreNoFrames() const
{
    return m_frameQueue.isEmpty();
}

void PlaybackManager::popFrame()
{
    m_frameQueue.pop();

}

std::pair<int, std::pair<void *, int> > PlaybackManager::frontFrame()
{
    return m_frameQueue.front();
}

PlaybackManager::PlaybackManager()
    : QObject(nullptr),m_countDaqRawDataCompleted(0), m_isPlayback(false),m_playbackLoopSleep(500),m_isSingleStep(false)
{
}

bool PlaybackManager::isSingleStep()
{
    bool retVal = m_isSingleStep;
    m_isSingleStep = false;
    return retVal;
}

std::vector<int> PlaybackManager::count() const
{
    return m_countContainer;
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

void PlaybackManager::setPlaybackSpeed(unsigned long speed)
{
    unsigned long maxSleep(1030);
    m_playbackLoopSleep = maxSleep - speed;
}

int PlaybackManager::frameIndex() const
{
    return m_frameIndex;
}

int PlaybackManager::countDaqRawDataCompleted() const
{
    return m_countDaqRawDataCompleted;
}

int PlaybackManager::queueSize() const
{
    return int(m_validFrameIndexCache.size());
}

void PlaybackManager::printCache()
{
    while(!m_validFrameIndexCache.empty()){
        LOG1(m_validFrameIndexCache.front())
        m_validFrameIndexCache.pop();
    }
}
