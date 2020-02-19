#ifndef FILEDAQ_H
#define FILEDAQ_H

#include "idaq.h"
#include "octFile.h"
#include "defaults.h"

#include <memory>

class DSPGPU;
class Producer;

class FileDaq : public IDAQ
{
public:
    FileDaq();
    ~FileDaq() override;

    void init( void ) override;
    void run( void ) override;

    void stop( void ) override;
    void pause( void ) override;
    void resume( void) override;

    QString getDaqLevel() override;
    long getRecordLength() const override;

    bool configure( void ) override;
    bool getData( void ) override;

    void enableAuxTriggerAsTriggerEnable( bool ) override;

private:
    bool m_isConfigured{false};
    std::unique_ptr<DSPGPU>  m_dsp{nullptr};
    std::unique_ptr<Producer>  m_producer{nullptr};

    QString m_daqLevel;

    long m_recordLenght{0};
    bool m_isRunning{false};
    int m_count1{0};
    int m_count2{0};
};

#endif // FILEDAQ_H
