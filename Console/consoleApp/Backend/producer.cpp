#include "producer.h"
#include "theglobals.h"
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
//    auto tg = TheGlobals::instance();
    int count{0};
    m_producerIsRunning = true;
    while(m_producerIsRunning)
    {
        msleep(1);
        if(sm->isSignalContainerEmpty()){
            sm->loadFftSignalBuffers();
//            LOG1(count)
            ++count;
        }
        QApplication::processEvents();
        yieldCurrentThread();

    }
}

void Producer::stop()
{
    m_producerIsRunning = false;
}
