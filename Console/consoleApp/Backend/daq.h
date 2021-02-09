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
    void setSubsampling(int speed) override;

    IDAQ* getSignalSource() override;

    bool shutdownDaq() override;

private:
    bool startDaq();
    void setSubSamplingFactor();
    bool getData(new_image_callback_data_t data);
    void initLogLevelAndDecimation();
    void logRegisterValue(int line, int reg);

    void logAxErrorVerbose(int line, AxErr e, int count = 0);

    static void NewImageArrived(new_image_callback_data_t data, void* user_ptr);

private:
    int m_frameNumber{FRAME_BUFFER_SIZE - 1};
    AOChandle session{nullptr};
    QElapsedTimer imageFrameTimer;
    char axMessage[256];

    int m_daqDecimation{0};
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
