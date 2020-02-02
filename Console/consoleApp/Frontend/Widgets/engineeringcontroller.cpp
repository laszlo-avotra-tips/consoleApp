#include <QTextStream>

#include "engineeringcontroller.h"
#include "enginneringmodel.h"
#include "engineeringdialog.h"
#include "logger.h"
#include "playbackmanager.h"
#include "sledsupport.h"
#include "signalmanager.h"


EngineeringController::EngineeringController(QWidget *parent)
    : QObject(parent),m_view(nullptr), m_model(nullptr),m_isGeometrySet(false)
{
    m_view = new EngineeringDialog(parent);
    m_model = new EngineeringModel(parent);

    connect(m_view, SIGNAL(playbackSpeedChanged(int)), m_model, SLOT(setPlaybackSpeed(int)));
    connect(m_view, SIGNAL(voaAttenuationChanged(int)), m_model, SLOT(setVoaAttenuation(int)));
    connect(m_view, SIGNAL(motorSpeedChanged(int)), m_model, SLOT(setMotorSpeed(int)));
    connect(m_view, SIGNAL(fileNameChanged(const QString&)), m_model, SLOT(setFileName(const QString&)));
    connect(m_view, SIGNAL(isLaserOnChanged(bool)), m_model, SLOT(setLaserOn(bool)));
    connect(m_view, SIGNAL(isMotorOnChanged(bool)), m_model, SLOT(setMotorPowerOn(bool)));
    connect(&m_statTimer, SIGNAL(timeout()), this, SLOT(updateStat()));
    connect(m_view, SIGNAL(loadFrame()), this, SLOT(loadFrameBuffers()));
    connect(m_view, SIGNAL(saveFrame()), this, SLOT(saveFrameBuffers()));
    connect(m_view, SIGNAL(singleStep()), PlaybackManager::instance(), SLOT(singleStep()));

    connect(m_view, SIGNAL(playbackStartStop(bool)), this, SLOT(playbackStartStopCommand(bool)));
    m_view->hide();

    connect(m_model, SIGNAL(playbackSpeedChanged(int)), this, SLOT(setPlaybackSpeed(int)));

    connect(PlaybackManager::instance(), SIGNAL(countChanged(int, int)),
            m_view, SLOT(countChanged(int, int)));

    connect(PlaybackManager::instance(), SIGNAL(rawDataBuffersAvailable(int)),
            m_view, SLOT(setFramesAvailable(int)));

    SledSupport& sledSp = SledSupport::Instance();
    connect(&sledSp, SIGNAL(speedChanged(int)), m_view, SLOT(setMotorSpeed(int)));

    connect(m_view, SIGNAL(saveDataToFile()), this, SLOT(handleSaveDataToFile()));

    m_view->signalsConnected();

#ifdef QT_NO_DEBUG
    m_statTimer.start(200);
#endif
}

void EngineeringController::setViewPosition(int x, int y)
{
    if(!m_isGeometrySet){
        m_view->setGeometry( x,
                             y,
                             m_view->width(),
                             m_view->height() );
        m_isGeometrySet = true;
    }

}

void EngineeringController::showOrHideView(bool isShown)
{
    if(isShown){
        m_view->show();
    } else{
        m_view->hide();
    }
}

void EngineeringController::updateStat()
{
    QString msg;
    QTextStream qt(&msg);
    qt << "Queued rawData: " <<  PlaybackManager::instance()->queueSize();
    qt << ", rawData Processed: " << PlaybackManager::instance()->countOfRawDataBuffersProcessed();
    qt << ", rawData index: " << PlaybackManager::instance()->frameIndex();

    if(!PlaybackManager::instance()->isPlayback()){
        m_view->setStatMsg(msg);
    }
}

void EngineeringController::saveFrameBuffers()
{
    const auto& fn =  m_model->getFileName();

    m_model->setLaserOn(false);

    PlaybackManager::instance()->saveBuffer(fn);

    m_model->setLaserOn(true);

}

void EngineeringController::loadFrameBuffers()
{
    const auto& fn =  m_model->getFileName();

    if(!fn.isEmpty()){
//        m_model->setLaserOn(false);

        PlaybackManager::instance()->loadBuffer(fn);
    }
}

void EngineeringController::playbackStartStopCommand(bool isStart)
{
    LOG1(isStart);
    if(isStart){
        startPlayback();
    }else{
        stopPlayback();
    }

}

void EngineeringController::setPlaybackSpeed(int speed)
{
    PlaybackManager::instance()->setPlaybackSpeed(speed);
}

void EngineeringController::handleSaveDataToFile()
{
    QTimer::singleShot(100,this,SLOT(saveDataToFile() ) ) ;
}

void EngineeringController::saveDataToFile()
{
    SignalManager::instance()->saveSignal();
}

void EngineeringController::startPlayback()
{
    PlaybackManager::instance()->startPlayback();
}

void EngineeringController::stopPlayback()
{
    PlaybackManager::instance()->stopPlayback();
}
