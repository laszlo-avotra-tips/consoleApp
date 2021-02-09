#ifndef DAQ_H
#define DAQ_H

#include <QtCore>
#include <QThread>
#include <QElapsedTimer>
#include "scanconversion.h"
#include "octFile.h"
#include "AxsunOCTCapture.h"
#include <cstdint>


class DAQ: public QObject
{
    Q_OBJECT

public:
    static DAQ* instance();
    void initDaq( void );
    void setSubsampling(int speed);

    DAQ* getSignalSource();

    bool shutdownDaq();

private:
    DAQ();
    ~DAQ();
    bool startDaq();
    void setSubSamplingFactor();
    bool getData(new_image_callback_data_t data);
    void initLogLevelAndDecimation();
    void logRegisterValue(int line, int reg);

    void logAxErrorVerbose(int line, AxErr e, int count = 0);

    static void NewImageArrived(new_image_callback_data_t data, void* user_ptr);

private:
    static DAQ* m_instance;
    int m_frameNumber{FRAME_BUFFER_SIZE - 1};
    AOChandle session{nullptr};
    QElapsedTimer imageFrameTimer;
    char axMessage[256];

    int m_daqDecimation{0};
    int m_daqLevel{0};
    int m_daqCount{0};
    bool m_disableRendering{false};

    const int m_subsamplingThreshold{1000};
    int m_subsamplingFactor{2};
    int m_numberOfConnectedDevices {0};

    uint32_t m_droppedPackets{0};
    uint32_t m_missedImagesCountAccumulated{0};
    uint32_t m_lastDroppedPacketCount{0};

};

#endif // DAQ_H
