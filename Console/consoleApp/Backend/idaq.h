#ifndef IDAQ_H
#define IDAQ_H

#include <QThread>
#include <QString>
#include "octFile.h"

class IDAQ : public QThread
{
    Q_OBJECT

public:
    virtual ~IDAQ(){}

    virtual void stopDaq() = 0;
    virtual void pauseDaq() = 0;
    virtual void resumeDaq() = 0;
    virtual void initDaq() = 0;
    virtual void setSubsampling(int speed) = 0;

    virtual IDAQ* getSignalSource() { return nullptr;}

    virtual bool configureDaq( void ) = 0;
    virtual bool shutdownDaq() = 0;

signals:
    void sendWarning( QString );
    void sendError( QString );
    void setBlackLevel( int );
    void setWhiteLevel( int );
    void frameRate( int );
    void handleDisplayAngle( float );
    void signalDaqResetToFrontend( void );
    void attenuateLaser( bool );
    void updateSector(OCTFile::OctData_t*);
    void notifyAcqData();
};

#endif // IDAQ_H
