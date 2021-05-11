#include "displayThread.h"
#include "logger.h"
#include "signalmodel.h"
#include "mainScreen.h"
#include <QElapsedTimer>

DisplayThread::DisplayThread(MainScreen *ms) : m_ms(ms)
{

}

void DisplayThread::run()
{
    while(true){
        if(m_ms) {
//            m_ms->updateImage();
            msleep(1);
            emit update();
        }
        msleep(50);
    }
}

void DisplayThread::run2()
{
    auto sm = SignalModel::instance();
    int index{-1};
    QElapsedTimer time;
    time.start();
    while(true){
        msleep(1);
        ++m_count;
        yieldCurrentThread();
        int qIndex = sm->renderingQueueIndex();
        if(qIndex > 0 && index != qIndex){
            index = qIndex;
            auto p = sm->getOctData(index);
            int frameNumber{-1};

            if(p){
                frameNumber = p->frameNumber;
                m_ms->presentData(p);
            }
            auto deltaT = time.elapsed();
            LOG4(m_count, qIndex, frameNumber, deltaT);
            time.restart();
        }
    }
}
