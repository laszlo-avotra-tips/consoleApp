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
    bool isPlayback() const;
    void startPlayback();
    void stopPlayback();
    void setPlaybackLoopDelay(int speed);

    unsigned long playbackLoopSleep() const;

    void setCount(int count, int index);

    bool isSingleStep();

signals:
    void countChanged(int count, int index);

public slots:
    void singleStep();

private:
    PlaybackManager();

    static PlaybackManager* m_instance;
    bool m_isPlayback;
    unsigned long m_playbackLoopSleep{1000};
    bool m_isSingleStep;
};

#endif // PLAYBACKMANAGER_H
