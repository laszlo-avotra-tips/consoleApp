#include <QFile>
#include <QDataStream>

#include "playbackmanager.h"
#include "logger.h"


PlaybackManager* PlaybackManager::m_instance(nullptr);

PlaybackManager *PlaybackManager::instance()
{
    if(!m_instance){
        m_instance = new PlaybackManager();
    }
    return m_instance;
}

bool PlaybackManager::isPlayback() const
{
    return m_isPlayback;
}

PlaybackManager::PlaybackManager()
    : QObject(nullptr), m_isPlayback(true),m_isSingleStep(false)
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
    emit countChanged(count, index);
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

void PlaybackManager::setPlaybackLoopDelay(int speed)
{
    const int maxSleep(1001);
//    m_playbackLoopSleep = 4 * log(maxSleep - quint32(speed));
    m_playbackLoopSleep = std::abs(maxSleep - speed);
}
