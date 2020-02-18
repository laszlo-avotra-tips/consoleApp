#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include <map>
#include <vector>
#include <queue>
#include <QObject>
#include "octFile.h"


class PlaybackManager : public QObject
{
    Q_OBJECT

public:
    static PlaybackManager* instance();

    void setPlayerOn(bool isOn);
    void setRecorderOn(bool isOn);
    void addRawDataBuffer(int index, void* fb, ulong size);
    void recordRawData(int index, int count);
    void retrieveRawData(int index, const void* buffer);
    void saveBuffer(const QString& fn);
    void loadBuffer(const QString& fn);
    bool isPlayback() const;

    int countOfRawDataBuffersProcessed() const;
    int queueSize() const;

    int frameIndex() const;
    void startPlayback();
    void stopPlayback();
    void setPlaybackSpeed(int speed);

    unsigned long playbackLoopSleep() const;

    bool isInputQueue() const;
    bool EnqueueBuffer(int index);
    bool findInputBuffer(int index, void*& dataBuffer);
    bool findDisplayBuffer(int index, OCTFile::FrameData_t*& frameData);
    void inputProcessingDone(int index);
    bool isFrameQueue() const;
    void frameReady(int index);

    void setCount(int count, int index);

    bool isSingleStep();

signals:
    void countChanged(int count, int index);
    void rawDataBuffersAvailable(int count);

public slots:
    void singleStep();

private:
    PlaybackManager();

    static PlaybackManager* m_instance;

    std::map<int, std::pair<void*, int>> m_rawDataBufferContainer;
    std::queue<int> m_dawDataBufferIndexQueue;
    int m_countOfRawDataProcessed;
    int m_frameIndex;
    bool m_isPlayback;
    unsigned long m_playbackLoopSleep;
    std::vector<int> m_countContainer;
    bool m_isSingleStep;
};

#endif // PLAYBACKMANAGER_H
