#ifndef IDAQ_H
#define IDAQ_H

#include <QThread>
#include <QString>
#include "octFile.h"

class IDAQ : public QObject
{
    Q_OBJECT

public:
    virtual ~IDAQ(){}

    virtual void initDaq() = 0;
    virtual void setSubsampling(int speed) = 0;

    virtual IDAQ* getSignalSource() { return nullptr;}
    virtual bool shutdownDaq() = 0;

signals:
    void sendWarning( QString );
    void sendError( QString );
    void setBlackLevel( int );
    void setWhiteLevel( int );
    void updateSector(OCTFile::OctData_t*);
};

#endif // IDAQ_H
