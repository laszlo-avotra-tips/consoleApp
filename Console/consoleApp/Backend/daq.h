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
#include <map>

class MainScreen;

using ForceTriggerTimeoutTable = std::map<int,int>;

class DAQ: public IDAQ
{
    Q_OBJECT

public:
    DAQ(MainScreen* ms);
    ~DAQ();
    void initDaq( void ) override;
    void setSubsamplingAndForcedTrigger(int speed) override;

    IDAQ* getSignalSource() override;

    bool shutdownDaq() override;

    virtual bool turnLaserOn() override;
    virtual bool turnLaserOff() override;
    bool startDaq() override;

private:
    void setSubSamplingFactor();
    void getData(new_image_callback_data_t data);
    void initLogLevelAndDecimation();
    void logRegisterValue(int line, int reg);

    void logAxErrorVerbose(int line, AxErr e, int count = 0);
    bool setLaserEmissionState(uint32_t emission_state); // emission_state =1 enables laser emission, =0 disables laser emission.

    static void NewImageArrived(new_image_callback_data_t data, void* user_ptr);

private:
    const ForceTriggerTimeoutTable m_forceTriggerTimeoutTable
    {//   rpm  | timeout
        { 2000 ,   12   },
        { 1000 ,   23   },
        { 600  ,   20   },
        { 800  ,   15   }
    };
    const int m_framesUntilForceTrigDefault{24};

    int m_bufferNumber;
    AOChandle session{nullptr};
    QElapsedTimer imageFrameTimer;
    char axMessage[256];

    int m_daqDecimation{0};
    int m_callbackCount{0};

    const int m_subsamplingThreshold{1000};
    int m_subsamplingFactor{2};
    int m_numberOfConnectedDevices {0};

    uint32_t m_droppedPackets{0};
    uint32_t m_missedImagesCountAccumulated{0};
    uint32_t m_lastDroppedPacketCount{0};

    unsigned long m_frameGoodCount{0};
    unsigned long m_frameBadCount{0};
    unsigned long m_imageNumber{0};

    QElapsedTimer m_callbackTimer;
    QElapsedTimer m_dataTimer;
    int m_callbackTime{0};
    int m_dataTime{0};
    MainScreen* m_mainScreen{nullptr};

};

#endif // DAQ_H
