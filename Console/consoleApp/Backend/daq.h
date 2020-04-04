#ifndef DAQ_H
#define DAQ_H

#include <QtCore>
#include <QThread>
#include <QTime>
#include "scanconversion.h"
#include "octFile.h"
#include "AxsunOCTCapture.h"
#include "idaq.h"
#include <cstdint>


class DAQ: public IDAQ
{
    Q_OBJECT

public:
    DAQ();
    ~DAQ();
    void init( void ) override;
    void run( void ) override;

    void stop( void ) override;
    void pause( void ) override;
    void resume( void) override;

    QString getDaqLevel() override;
    long getRecordLength() const override;

    bool configure( void ) override;

    void enableAuxTriggerAsTriggerEnable( bool ) override;
    IDAQ* getSignalSource() override;

    bool getData();
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

//protected:
//    void run( void );

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
