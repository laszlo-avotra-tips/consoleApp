#include "displayThread.h"
#include "logger.h"
#include "signalmodel.h"
#include <QElapsedTimer>

DisplayThread::DisplayThread()
{

}

void DisplayThread::run()
{
    auto sm = SignalModel::instance();
    int index{-1};
    QElapsedTimer time;
    time.start();
    while(true){
        yieldCurrentThread();
        msleep(1);
        ++m_count;
        yieldCurrentThread();
        if(sm->renderingQueueIndex() > 0 && index != sm->renderingQueueIndex()){
            index = sm->renderingQueueIndex();
            auto deltaT = time.elapsed();
            LOG4(m_count, sm->renderingQueueIndex(), priority(), deltaT);
            time.restart();
        }
    }
}
