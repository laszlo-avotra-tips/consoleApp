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
        if(sm->renderingQueueIndex() > 0 && index != sm->renderingQueueIndex()){
            index = sm->renderingQueueIndex();
            LOG3(m_count, sm->renderingQueueIndex(), priority());
        }
    }
}
