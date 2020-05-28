#ifndef BACKEND_H
#define BACKEND_H

#include <QWidget>
#include <QCommandLineOption>
#include "initialization.h"

class Backend : public QWidget
{
    Q_OBJECT
public:
    explicit Backend(QWidget *parent = nullptr);

    bool isPhysicianScreenAvailable();

    void parseOptions( QCommandLineOption &options, QStringList args );

signals:

private:
    Initialization m_init;
};

#endif // BACKEND_H
