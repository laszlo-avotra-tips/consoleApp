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
        int qIndex = sm->renderingQueueIndex();
        if(qIndex > 0 && index != qIndex){
            index = qIndex;
            auto data = sm->getOctData(index);
            auto frameNumber = data->frameNumber;
            auto deltaT = time.elapsed();
            LOG4(m_count, qIndex, frameNumber, deltaT);
            time.restart();
        }
    }
}
