#ifndef DISPLAYTHREAD_H
#define DISPLAYTHREAD_H

#include <QThread>
#include <QObject>

class DisplayThread : public QThread
{
public:
    DisplayThread();
    void run() override;

private:
    int m_count{0};
};

#endif // DISPLAYTHREAD_H
