#ifndef ENGINNERINGMODEL_H
#define ENGINNERINGMODEL_H

#include <QObject>
#include <QString>

class EngineeringModel : public QObject
{
    Q_OBJECT
public:
    explicit EngineeringModel(QObject *parent = nullptr);

    const QString& getFileName() const;

    int getPlaybackSpeed() const;

signals:
    void laserOnChanged(bool isLaserOn);
    void motorOnChanged(bool isMotorOn);
    void playbackSpeedChanged(int speed);

public slots:
    void setPlaybackSpeed(int speed);
    void setVoaAttenuation(int attenuation);
    void setMotorSpeed(int speed);
    void setFileName(const QString& fn);
    void setLaserOn(bool isOn);
    void setMotorPowerOn(bool isOn);

private:
    QString m_fileName;
    bool m_isLaserOn;
    bool m_isMotorPowerOn;
    int m_frameCount;
    int m_playbackSpeed;
    int m_voaAttenuation;
    int m_motorSpeed;
};

#endif // ENGINNERINGMODEL_H
