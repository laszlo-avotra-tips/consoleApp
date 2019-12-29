#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include <map>
#include <vector>
#include <queue>
#include "framequeue.h"
#include <QObject>


class PlaybackManager : public QObject
{
    Q_OBJECT

public:
    static PlaybackManager* instance();

    void setPlayerOn(bool isOn);
    void setRecorderOn(bool isOn);
    void addFrameBuffer(int index, void* fb, ulong size);
    void recordFrameData(int index, int count);
    void retrieveFrameData(int index, const void* buffer);
    void saveBuffer(const QString& fn);
    void loadBuffer(const QString& fn);
    bool isPlayback() const;
    bool thereAreNoFrames() const;
    std::pair<int,std::pair<void*, int>> frontFrame();
    void popFrame();

    int countDaqRawDataCompleted() const;
    int queueSize() const;
    void printCache();

    int frameIndex() const;
    void startPlayback();
    void stopPlayback();
    void setPlaybackSpeed(int speed);

    unsigned long playbackLoopSleep() const;

    std::vector<int> count() const;
    void setCount(int count, int index);

    bool isSingleStep();

signals:
    void countChanged(int count, int index);
    void framesAvailable(int count);

public slots:
    void singleStep();

private:
    PlaybackManager();

    static PlaybackManager* m_instance;

    std::map<int, std::pair<void*, int>> m_frameBufferContainer;
    std::queue<int> m_validFrameIndexCache;
    int m_countDaqRawDataCompleted;
    int m_frameIndex;
    bool m_isPlayback;
    FrameQueue m_frameQueue;
    unsigned long m_playbackLoopSleep;
    std::vector<int> m_countContainer;
    bool m_isSingleStep;
};

#endif // PLAYBACKMANAGER_H
