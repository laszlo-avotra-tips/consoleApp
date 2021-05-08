#include "displayThread.h"
#include "logger.h"

DisplayThread::DisplayThread()
{

}

void DisplayThread::run()
{
    while(true){
        sleep(1);
        ++m_count;
        LOG1(m_count);
    }
}
