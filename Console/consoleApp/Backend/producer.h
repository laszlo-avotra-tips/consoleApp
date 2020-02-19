#ifndef PRODUCER_H
#define PRODUCER_H

#include <QThread>

class Producer : public QThread
{
public:
    Producer();
    ~Producer() override;

    void run() override;
    void stop();

private:
    bool m_producerIsRunning{false};
};

#endif // PRODUCER_H
