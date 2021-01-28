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
    void initDaq( void ) override;
    void run( void ) override;
    void setSubsampling(int speed) override;

    IDAQ* getSignalSource() override;

    bool isRunning;
    QElapsedTimer frameTimer;
    QElapsedTimer fileTimer;
    bool shutdownDaq() override;

public slots:
    void setLaserDivider();
    void setDisplay( float, int );

private:
    bool getData(new_image_callback_data_t data);
    void logDecimation();
    void logRegisterValue(int line, int reg);

    void logAxErrorVerbose(int line, AxErr e, int count = 0);
    static void NewImageArrived(new_image_callback_data_t data, void* user_ptr);

private:
    AOChandle session = NULL;
    char axMessage[256];
    uint32_t lastImageIdx;
    int missedImgs;
    bool startDaq();
    uint16_t lastPolarLineIndexEntered;
    int m_daqDecimation{0};
    int m_imageDecimation{0};
    int m_daqLevel{0};
    int m_daqCount{0};
    const int m_subsamplingThreshold{1000};
    int m_subsamplingFactor{2};
    int m_numberOfConnectedDevices {0};
    uint32_t m_droppedPackets{0};
    uint32_t m_badCountAcc{0};

};

#endif // DAQ_H
