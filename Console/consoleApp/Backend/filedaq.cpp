#include <QApplication>

#include "filedaq.h"
#include "logger.h"
#include "playbackmanager.h"
#include "dspgpu.h"
#include "daqSettings.h"
#include "deviceSettings.h"
#include "theglobals.h"
#include "signalmanager.h"
#include "signalmodel.h"


FileDaq::FileDaq()
{
    DaqSettings &settings = DaqSettings::Instance();

    if( !settings.init() )
    {
        LOG( WARNING, "Unable to load DAQ Settings." )
    }
}

FileDaq::~FileDaq()
{
}

void FileDaq::init()
{
    m_isRunning = false;

    // Create the DSP
    m_dsp = std::make_unique<DSPGPU>();

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

    m_dsp->init();

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

    // prevent multiple, simultaneous starts
    if( !m_isRunning )
    {
        deviceSettings &devSettings = deviceSettings::Instance();
        bool isHighSpeedDevice = devSettings.current()->isHighSpeed();

        qDebug() << "Thread: DAQ::run start";

        m_dsp->start();

        m_isRunning = true;

        // set the brightness and contrast when the daq starts up
        if( isHighSpeedDevice )
        {
            auto signalModelInstance = SignalModel::instance();
            signalModelInstance->setBlackLevel( BrightnessLevels_HighSpeed.defaultValue );
            signalModelInstance->setWhiteLevel( ContrastLevels_HighSpeed.defaultValue );
        }
    }

    const auto& pmi = PlaybackManager::instance();
    auto smi = SignalManager::instance();
    const auto * const smiReadOnly = SignalManager::instance();
    while( m_isRunning )
    {
        if(pmi->isPlayback() || pmi->isSingleStep())
        {
            ++videoFrameCount;
            pmi->setCount(videoFrameCount, m_count2);
            if(m_count2 == 0) {
                smi->open();
            }
            smi->loadFftSignalBuffers(m_count2);
            m_dsp->readInputBuffers(smiReadOnly->getFftImagDataPointer(), smiReadOnly->getFftRealDataPointer());
            m_dsp->processData(m_count2);
            ++m_count2;
            if(m_count2 == FRAME_BUFFER_SIZE)
            {
                smi->close();
                m_count2 = 0;
            }
            if(pmi->isPlayback()){
                msleep(pmi->playbackLoopSleep());
            }
        }

        ++m_count1;

        // threads do not handle events by default (timer expiration). Do so explicitly.
        QApplication::processEvents();
        yieldCurrentThread();
        msleep(1);
    }

    LOG3(m_count1,m_count2,m_count1-m_count2)
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
