#include <QApplication>

#include "filedaq.h"
#include "logger.h"
#include "playbackmanager.h"
#include "dspgpu.h"
#include "daqSettings.h"
#include "deviceSettings.h"
#include "theglobals.h"


FileDaq::FileDaq(): m_isConfigured(false),m_dsp(nullptr),
    m_recordLenght(0),m_isRunning(true),m_count1(0),m_count2(0)
{
    DaqSettings &settings = DaqSettings::Instance();

    if( !settings.init() )
    {
        LOG( WARNING, "Unable to load DAQ Settings." )
    }

    LOG2(m_count1, m_count2)
}

FileDaq::~FileDaq()
{
    delete m_dsp;
}

void FileDaq::init()
{
    m_isRunning = true;
    LOG2(m_count1, m_count2)

    // Create the DSP
    m_dsp = new DSPGPU();

    if( !m_dsp )
    {
        // fatal error
        emit sendError( tr( "DSP thread could not be created." ) );
    }

    if( !configure() )
    {
        // fatal error
        emit sendError( tr( "DAQ hardware could not be initialized." ) );
    }

    quint8  bitsPerSample        = 16;
    quint32 bytesPerSample   = ( bitsPerSample + 7 ) / 8;
    quint32 preTriggerSamples = DaqSettings::Instance().getPreDepth();
    quint32 postTriggerSamples = DaqSettings::Instance().getRecordLength() - preTriggerSamples;
    quint32 samplesPerRecord = preTriggerSamples + postTriggerSamples;
    quint32 bytesPerRecord   = bytesPerSample * samplesPerRecord;

    quint32 recordsPerBuffer = quint32(deviceSettings::Instance().current()->getLinesPerRevolution());

    quint32 channelCount = 1;
    quint32 bytesPerBuffer = bytesPerRecord * recordsPerBuffer * channelCount;

    // initialize the DSP thread
    m_dsp->init( DaqSettings::Instance().getRecordLength(),
               quint16(deviceSettings::Instance().current()->getLinesPerRevolution()),
               bytesPerRecord,
               bytesPerBuffer, int(channelCount) );

    emit attenuateLaser( false );
}

bool FileDaq::configure()
{
    m_isConfigured = true;

    return m_isConfigured;
}

void FileDaq::run()
{
    while( !m_isConfigured ) // XXX
    {
        yieldCurrentThread();
    }

    LOG1(m_isConfigured)

    // prevent multiple, simultaneous starts
    if( !m_isRunning )
    {
        deviceSettings &devSettings = deviceSettings::Instance();
        bool isHighSpeedDevice = devSettings.current()->isHighSpeed();

        qDebug() << "Thread: DAQ::run start";
        m_isRunning = true;

        if( isHighSpeedDevice )
        {
            // Start the DSP thread
            m_dsp->start();
        }


        // set the brightness and contrast when the daq starts up
        if( isHighSpeedDevice )
        {
            setBlackLevel( BrightnessLevels_HighSpeed.defaultValue );
            setWhiteLevel( ContrastLevels_HighSpeed.defaultValue );
        }
        else
        {
            setBlackLevel( BrightnessLevels_LowSpeed.defaultValue );
            setWhiteLevel( ContrastLevels_LowSpeed.defaultValue );
        }
    }

    while( m_isRunning )
    {
        if(PlaybackManager::instance()->isPlayback() || PlaybackManager::instance()->isSingleStep())
        {
            auto tgi = TheGlobals::instance();
            tgi->updateGDaqRawData_idx();
            tgi->incrementGDaqRawDataCompleted();
            int index = tgi->getGDaqRawData_idx();
            ++m_count2;
            m_dsp->processData();
            PlaybackManager::instance()->setCount(m_count2, index);
            if(PlaybackManager::instance()->isPlayback()){
                msleep(PlaybackManager::instance()->playbackLoopSleep());
            }
//            LOG2(tgi->getGDaqRawData_idx(), tgi->getGFrameCounter())
        }

        ++m_count1;

        // threads do not handle events by default (timer expiration). Do so explicitly.
        QApplication::processEvents();
        yieldCurrentThread();
        msleep(1);

//        if(m_count1 % 2000 == 0){
//            LOG3(m_count1,m_count2,m_count1-m_count2);
//        }

    }

    LOG3(m_count1,m_count2,m_count1-m_count2)
}

void FileDaq::stop()
{
    m_isRunning = false;
    LOG1(m_isRunning)
}

void FileDaq::pause()
{

}

void FileDaq::resume()
{

}

QString FileDaq::getDaqLevel()
{
    return m_daqLevel;
}

long FileDaq::getRecordLength() const
{
    return m_recordLenght;
}

bool FileDaq::getData()
{
    auto tgi = TheGlobals::instance();
    tgi->updateGDaqRawData_idx();
    tgi->incrementGDaqRawDataCompleted();

    return true;
}

void FileDaq::enableAuxTriggerAsTriggerEnable(bool)
{

}
