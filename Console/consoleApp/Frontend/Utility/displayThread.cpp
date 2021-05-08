#include "displayThread.h"
#include "logger.h"
#include "signalmodel.h"

DisplayThread::DisplayThread()
{

}

void DisplayThread::run()
{
    auto sm = SignalModel::instance();
    int index{-1};
    while(true){
        yieldCurrentThread();
        msleep(1);
        ++m_count;
        yieldCurrentThread();
        if(index != sm->renderingQueueIndex()){
            index = sm->renderingQueueIndex();
            LOG2(m_count, sm->renderingQueueIndex());
        }
    }
}
