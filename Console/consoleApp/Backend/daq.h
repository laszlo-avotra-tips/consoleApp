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
    bool getData(new_image_callback_data_t data);
    void logDecimation();
    void logRegisterValue(int line, int reg);

    void logAxErrorVerbose(int line, AxErr e);
    static void NewImageArrived(new_image_callback_data_t data, void* user_ptr);

private:
    AOChandle session = NULL;
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
