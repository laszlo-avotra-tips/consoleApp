#include "producer.h"
#include "signalmanager.h"
#include "logger.h"

#include <QTime>
#include <QApplication>

Producer::Producer()
{
    SignalManager::instance()->open();
}

Producer::~Producer()
{
    stop();
    wait(20);
    SignalManager::instance()->close();
}

void Producer::run()
{
    auto sm = SignalManager::instance();
    int count{0};
    m_producerIsRunning = true;
    while(m_producerIsRunning)
    {
//        msleep(1);
        if(sm->isSignalQueueLengthLTE(2)){
            sm->loadFftSignalBuffers();
            ++count;
        }
        yieldCurrentThread();
    }
}

void Producer::stop()
{
    m_producerIsRunning = false;
}
