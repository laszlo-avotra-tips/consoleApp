#include "displayThread.h"
#include "logger.h"
#include "signalmodel.h"

DisplayThread::DisplayThread()
{

}

void DisplayThread::run()
{
    auto sm = SignalModel::instance();
    while(true){
        sleep(1);
        ++m_count;
        LOG2(m_count, sm->renderingQueueIndex());
    }
}
