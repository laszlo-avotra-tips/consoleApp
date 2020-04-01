#ifndef DAQ_H
#define DAQ_H

#include <QtCore>
#include <QThread>
#include <QTime>
#include "scanconversion.h"
#include "octFile.h"
#include "AxsunOCTCapture.h"

typedef unsigned char   uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned int    uint32_t;

class DAQ: public QThread
{
    Q_OBJECT

public:
    DAQ();
    ~DAQ();
    void stop( void ){ isRunning = false; }
    bool getData( bool isFirstReceived );
    bool isRunning;
    int generateSyntheticData( unsigned char *pSyntheticData );
    QTime frameTimer;
    QTime fileTimer;

signals:
    void updateSector();
    void fpsCount( int );
    void linesPerFrameCount( int );
    void missedImagesCount( int );
    void setDisplayAngle( float, int );

public slots:
    void setLaserDivider( int divider );
    void setDisplay( float, int );

protected:
    void run( void );

private:
    AOChandle session = NULL;
    AxErr axRetVal = NO_AxERROR;
    ScanConversion *scanWorker;
    char axMessage[256];
    uint32_t lastImageIdx;
    int missedImgs;
    bool startDaq();
    bool shutdownDaq();
    int lapCounter;
    uint16_t lastPolarLineIndexEntered;

};

#endif // DAQ_H
