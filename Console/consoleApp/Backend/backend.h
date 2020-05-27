#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QCommandLineOption>
#include "initialization.h"

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(int appId, int argc, char **argv, QObject *parent = nullptr);

    bool isPhysicianScreenAvailable();

    int setupCase(bool);
    void parseOptions( QCommandLineOption &options, QStringList args );

signals:

private:
    Initialization m_init;
};

#endif // BACKEND_H
