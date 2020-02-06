#include <QApplication>

#include "filedaq.h"
#include "logger.h"
#include "playbackmanager.h"
#include "dspgpu.h"
#include "daqSettings.h"
#include "deviceSettings.h"
//#include "util.h"
#include "theglobals.h"
#include "signalmanager.h"


FileDaq::FileDaq(): m_isConfigured(false),m_dsp(nullptr),
    m_recordLenght(0),m_isRunning(true),m_count1(0),m_count2(0)
{
    DaqSettings &settings = DaqSettings::Instance();

    if( !settings.init() )
    {
        LOG( WARNING, "Unable to load DAQ Settings." );
    }
}

FileDaq::~FileDaq()
{
    delete m_dsp;
}

void FileDaq::init()
{
    m_isRunning = false;

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
    int bytesPerSample   = ( bitsPerSample + 7 ) / 8;
    quint32 preTriggerSamples = DaqSettings::Instance().getPreDepth();
    quint32 postTriggerSamples = DaqSettings::Instance().getRecordLength() - preTriggerSamples;
    quint32 samplesPerRecord = preTriggerSamples + postTriggerSamples;
    int bytesPerRecord   = bytesPerSample * int(samplesPerRecord);

    int recordsPerBuffer = deviceSettings::Instance().current()->getLinesPerRevolution();

    int bytesPerBuffer = bytesPerRecord * recordsPerBuffer;

    // initialize the DSP thread
    m_dsp->init( DaqSettings::Instance().getRecordLength(),
               quint16(deviceSettings::Instance().current()->getLinesPerRevolution()),
               bytesPerRecord,
               bytesPerBuffer);

    emit attenuateLaser( false );
}

bool FileDaq::configure()
{
    m_isConfigured = true;

    return m_isConfigured;
}

void FileDaq::run()
{
    int videoFrameCount{0};
    while( !m_isConfigured ) // XXX
    {
        yieldCurrentThread();
    }

//lcv    LOG1(m_isConfigured);

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
            if(!SignalManager::instance()->isFftSource()){
                m_dsp->start();
            }
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
        if(PlaybackManager::instance()->isPlayback() )
        {
            ++videoFrameCount;
            PlaybackManager::instance()->setCount(videoFrameCount, m_count2);
            if(!SignalManager::instance()->isFftSource()){
                if(PlaybackManager::instance()->EnqueueBuffer(m_count2)){
                    m_dsp->processData(m_count2);
                    SignalManager::instance()->saveSignal(m_count2);
                }
            }else{
                if(m_count2 == 0) SignalManager::instance()->open();
                SignalManager::instance()->loadSignal(m_count2);
                m_dsp->loadFftOutMemoryObjects();
                m_dsp->processData(m_count2);
            }
            ++m_count2;
            if(m_count2 == FRAME_BUFFER_SIZE)
            {
                SignalManager::instance()->close();
                m_count2 = 0;
            }
            if(PlaybackManager::instance()->isPlayback()){
                msleep(PlaybackManager::instance()->playbackLoopSleep());
            }
        }

        if(PlaybackManager::instance()->isSingleStep()){
            ++videoFrameCount;
            PlaybackManager::instance()->setCount(videoFrameCount, m_count2);
            if(!SignalManager::instance()->isFftSource()){
                if(PlaybackManager::instance()->EnqueueBuffer(m_count2)){
                    m_dsp->processData(m_count2);
                    SignalManager::instance()->saveSignal(m_count2);
                }
            }else{
                if(m_count2 == 0) SignalManager::instance()->open();
                SignalManager::instance()->loadSignal(m_count2);
                m_dsp->loadFftOutMemoryObjects();
                m_dsp->processData(m_count2);
            }
            ++m_count2;
            if(m_count2 == FRAME_BUFFER_SIZE)
            {
                SignalManager::instance()->close();
                m_count2 = 0;
            }
        }

        ++m_count1;

        // threads do not handle events by default (timer expiration). Do so explicitly.
        QApplication::processEvents();
        yieldCurrentThread();
        msleep(1);
    }

    LOG3(m_count1,m_count2,m_count1-m_count2);
}

void FileDaq::stop()
{
    m_isRunning = false;
//    LOG1(m_isRunning);
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
    tgi->updateRawDataIndex();
    tgi->incrementRawDataIndexCompleted();

    return true;
}

void FileDaq::enableAuxTriggerAsTriggerEnable(bool)
{

}
