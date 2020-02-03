#include "engineeringdialog.h"
#include "ui_engineeringdialog.h"
#include "logger.h"
#include <QFileDialog>
#include <QObject>
#include "signalmanager.h"


EngineeringDialog::EngineeringDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EngineeringDialog),m_count(0)
{
    ui->setupUi(this);
    LOG1(++m_count);

    connect(ui->sliderPlaybackSpeed, SIGNAL(valueChanged(int)), this, SIGNAL(playbackSpeedChanged(int)));
    connect(ui->sliderVoa, SIGNAL(valueChanged(int)), this, SIGNAL(voaAttenuationChanged(int)));
    connect(ui->sliderMotorSpeed, SIGNAL(valueChanged(int)), this, SIGNAL(motorSpeedChanged(int)));
    connect(SignalManager::instance(), SIGNAL(signalSaved(void)), this, SLOT(enableFileButtons(void)));
    connect(SignalManager::instance(), SIGNAL(signalLoaded(void)), this, SLOT(enableSingleStep(void)));

    ui->pushButtonSaveData->setEnabled(false);

    enableSingleStep();
}

EngineeringDialog::~EngineeringDialog()
{
    delete ui;
}

void EngineeringDialog::setStatMsg(const QString& msg)
{
    ui->labelStat->setText(msg);
    //    LOG1(msg);
}

void EngineeringDialog::signalsConnected()
{
    if(!ui->lineEdit->text().isEmpty()){
        emit fileNameChanged(ui->lineEdit->text());
        if(!SignalManager::instance()->isFftSource()){
            ui->pushButtonLoad->setEnabled(true);
            ui->sliderPlaybackSpeed->setEnabled(true);
        }
    }
}

void EngineeringDialog::setMotorSpeed(int val)
{
    ui->sliderMotorSpeed->setValue(val);
}

void EngineeringDialog::setFramesAvailable(int frameCount)
{
    LOG1(frameCount);
    QString msg("Frames for playback: ");
    msg += QString::number(frameCount);
    ui->labelFramesForPlayback->setText(msg);
    ui->checkBoxPlayback->setEnabled(true);
    if(!SignalManager::instance()->isFftSource()){
        ui->pushButtonSingleStep->setEnabled(true);
    }
}

void EngineeringDialog::on_checkBoxLaserOn_toggled(bool checked)
{
    ui->checkBoxLaserOff->setChecked(!checked);
    emit isLaserOnChanged(checked);
}

void EngineeringDialog::on_checkBoxLaserOff_toggled(bool checked)
{
    ui->checkBoxLaserOn->setChecked(!checked);
}

void EngineeringDialog::on_groupBoxMotor_toggled(bool isOn)
{
    LOG1(isOn);
    emit isMotorOnChanged(isOn);
    ui->sliderMotorSpeed->setEnabled(isOn);
}

void EngineeringDialog::on_pushButtonLoad_clicked()
{
    LOG1(++m_count);
    emit loadFrame();
//    ui->checkBoxPlayback->setEnabled(true);
}

void EngineeringDialog::on_pushButtonSave_clicked()
{
    LOG1(++m_count);
    emit saveFrame();
}

void EngineeringDialog::on_pushButtonFile_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, ("Open File"),
                                                      "/Avinger_System",
                                                      ("Config (*.data)"));
    if(!fn.isEmpty()) {
        ui->lineEdit->setText(fn);

        emit fileNameChanged(fn);
        ui->pushButtonLoad->setEnabled(true);
        ui->sliderPlaybackSpeed->setEnabled(true);
#ifdef QT_NO_DEBUG
        ui->pushButtonSave->setEnabled(true);
#endif
    }
}

void EngineeringDialog::on_lineEdit_textChanged(const QString &fn)
{
    emit fileNameChanged(fn);
}

void EngineeringDialog::on_checkBoxPlayback_toggled(bool checked)
{
    LOG1(checked);
    emit playbackStartStop(checked);
}

void EngineeringDialog::countChanged(int count, int index)
{
    QString msg;
    QTextStream qts(&msg);

    qts << "Frame count = " << count << ", Frame index = " << index;

    LOG2(count, index);
    ui->labelStat->setText(msg);
}

void EngineeringDialog::on_pushButtonSingleStep_clicked()
{
    emit singleStep();
    if(!SignalManager::instance()->isFftSource()){
        ui->pushButtonSaveData->setEnabled(true);
    }
}

void EngineeringDialog::on_pushButtonSaveData_clicked()
{
    setButtonEnabledStatusWhileSavingTheSignal(false);

    emit saveDataToFile();

    LOG0
}

void EngineeringDialog::enableFileButtons()
{
    setButtonEnabledStatusWhileSavingTheSignal(true);

    LOG0
}

void EngineeringDialog::setButtonEnabledStatusWhileSavingTheSignal(bool buttonStatus)
{
    LOG1(buttonStatus);
    ui->pushButtonFile->setEnabled(buttonStatus);
    ui->pushButtonSave->setEnabled(buttonStatus);
    ui->pushButtonSaveData->setEnabled(buttonStatus);

    if(!SignalManager::instance()->isFftSource()){
        ui->pushButtonSingleStep->setEnabled(buttonStatus);
        ui->pushButtonLoad->setEnabled(buttonStatus);
    }

    ui->checkBoxPlayback->setEnabled(buttonStatus);
}

void EngineeringDialog::enableSingleStep()
{
    if(SignalManager::instance()->isFftSource()){
        ui->pushButtonSingleStep->setEnabled(true);
        ui->pushButtonFile->setEnabled(false);
    }
}
