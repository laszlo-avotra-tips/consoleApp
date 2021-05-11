#ifndef DISPLAYTHREAD_H
#define DISPLAYTHREAD_H

#include <QThread>
#include <QObject>

class MainScreen;

class DisplayThread : public QThread
{
    Q_OBJECT

public:
    DisplayThread(MainScreen* ms);
    void run() override;
    void run2();

signals:
    void update();

private:
    int m_count{0};
    MainScreen * const m_ms{nullptr};
};

#endif // DISPLAYTHREAD_H
