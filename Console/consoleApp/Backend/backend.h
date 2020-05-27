#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include "initialization.h"

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(int appId, int argc, char **argv, QObject *parent = nullptr);

    bool isPhysicianScreenAvailable();

signals:

private:
    Initialization m_init;
};

#endif // BACKEND_H
