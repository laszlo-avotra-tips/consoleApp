#ifndef IDAQ_H
#define IDAQ_H

#include <QThread>
#include <QString>

class IDAQ : public QObject
{
    Q_OBJECT

public:
    virtual ~IDAQ(){}

    virtual void initDaq() = 0;
    virtual void setSubsamplingAndForcedTrigger(int speed) = 0;

    virtual IDAQ* getSignalSource() { return nullptr;}
    virtual bool shutdownDaq() = 0;
    virtual bool turnLaserOn() = 0;
    virtual bool turnLaserOff() = 0;
    virtual bool startDaq() = 0;

signals:
    void sendWarning( QString );
    void sendError( QString );
    void setBlackLevel( int );
    void setWhiteLevel( int );
};

#endif // IDAQ_H
