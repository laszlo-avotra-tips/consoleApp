#ifndef FILEDAQ_H
#define FILEDAQ_H

#include "idaq.h"
#include "AlazarApiType.h"
#include "octFile.h"
#include "defaults.h"

class DSP;

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
    bool m_isConfigured;
    DSP  *m_dsp;

    QString m_daqLevel;

    long m_recordLenght;
    bool m_isRunning;
    int m_count1;
    int m_count2;
};

#endif // FILEDAQ_H
