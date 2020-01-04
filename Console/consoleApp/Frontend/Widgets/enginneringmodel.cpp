#include "enginneringmodel.h"
#include "logger.h"
#include "playbackmanager.h"
#include "ioController.h"
#include "laser.h"
#include "sledsupport.h"
#include "deviceSettings.h"

EngineeringModel::EngineeringModel(QObject *parent) : QObject(parent)
    ,m_frameCount(0),m_playbackSpeed(0),m_voaAttenuation(0),m_motorSpeed(0)
{

}

const QString &EngineeringModel::getFileName() const
{
    return m_fileName;
}

void EngineeringModel::setPlaybackSpeed(int speed)
{
    m_playbackSpeed = speed;
    LOG1(m_playbackSpeed)
    emit playbackSpeedChanged(m_playbackSpeed);
}

void EngineeringModel::setVoaAttenuation(int attenuation)
{
    const double maxVoa(3.09);
    m_voaAttenuation = attenuation;

    const double voa(maxVoa*attenuation*0.01);

    ioController::Instance().setAnalogVoltageOut(voa);
}

void EngineeringModel::setMotorSpeed(int speed)
{
    m_motorSpeed = speed;
    const QString qSpeed(QString::number(m_motorSpeed));
    const QByteArray baSpeed(qSpeed.toStdString().c_str());

    SledSupport::Instance().setSledSpeed(baSpeed);
}

void EngineeringModel::setFileName(const QString &fn)
{
    m_fileName = fn;
    LOG1(m_fileName)
}

void EngineeringModel::setLaserOn(bool isOn)
{
    if(m_isLaserOn != isOn){
        m_isLaserOn = isOn;
        LOG1(m_isLaserOn)
        emit laserOnChanged(m_isLaserOn);
        if(m_isLaserOn){
            Laser::Instance().turnDiodeOn();
        }else{
            Laser::Instance().turnDiodeOff();
        }
    }
}

void EngineeringModel::setMotorPowerOn(bool isOn)
{
    LOG1(isOn)
    if(m_isMotorPowerOn != isOn){
        m_isMotorPowerOn = isOn;
        LOG1(m_isMotorPowerOn)
        emit motorOnChanged(m_isMotorPowerOn);

        if(!m_isMotorPowerOn){
            SledSupport::Instance().stopSled();
        } else {
            SledSupport::Instance().startSled();
        }
    }
}

int EngineeringModel::getPlaybackSpeed() const
{
    return m_playbackSpeed;
}
