#ifndef DAQ_H
#define DAQ_H

#include <QtCore>
#include <QThread>
#include <QElapsedTimer>
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
    void setSubsampling(int speed) override;

    void stop( void ) override;
    void pause( void ) override;
    void resume( void) override;

    QString getDaqLevel() override;
    long getRecordLength() const override;

    bool configure( void ) override;

    void enableAuxTriggerAsTriggerEnable( bool ) override;
    IDAQ* getSignalSource() override;

    bool getData();
    bool getData1();
    bool isRunning;
    int generateSyntheticData( unsigned char *pSyntheticData );
    QElapsedTimer frameTimer;
    QElapsedTimer fileTimer;

signals:
    void fpsCount( int );
    void linesPerFrameCount( int );
    void missedImagesCount( int );

public slots:
    void setLaserDivider();
    void setDisplay( float, int );

private:
    void sendToAdvacedView(const OCTFile::OctData_t& od, int frameNumber);
    void logDecimation();
    void logAxErrorVerbose(int line, AxErr e);

private:
    AOChandle session = NULL;
    AOChandle session0 = NULL;
    char axMessage[256];
    uint32_t lastImageIdx;
    int missedImgs;
    bool startDaq();
    bool shutdownDaq();
    int lapCounter;
    uint16_t lastPolarLineIndexEntered;
    int m_decimation{0};
    int m_count{0};
    const int m_subsamplingThreshold{1000};
    int m_subsamplingFactor{2};
    int m_numberOfConnectedDevices {0};

};

#endif // DAQ_H
