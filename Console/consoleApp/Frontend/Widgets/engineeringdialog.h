#ifndef ENGINEERINGDIALOG_H
#define ENGINEERINGDIALOG_H

#include <QWidget>
#include <QString>
#include <map>

namespace Ui {
class EngineeringDialog;
}

class EngineeringDialog : public QWidget
{
    Q_OBJECT

public:
    explicit EngineeringDialog(QWidget *parent = nullptr);
    ~EngineeringDialog();

    void setStatMsg(const QString& msg);

public slots:
    void setMotorSpeed(int val);
    void setFramesAvailable(int fa);

signals:
    void playbackSpeedChanged(int speed);
    void voaAttenuationChanged(int attenuation);
    void motorSpeedChanged(int speed);
    void fileNameChanged(const QString& fn);
    void isLaserOnChanged(bool isLaserOff);
    void isMotorOnChanged(bool isOn);
    void loadFrame();
    void saveFrame();
    void playbackStartStop(bool isStart);
    void singleStep();

private slots:
    void on_checkBoxLaserOn_toggled(bool checked);
    void on_checkBoxLaserOff_toggled(bool checked);
    void on_groupBoxMotor_toggled(bool arg1);
    void on_pushButtonLoad_clicked();
    void on_pushButtonSave_clicked();

    void on_pushButtonFile_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_checkBoxPlayback_toggled(bool checked);

    void countChanged(int count, int index);

    void on_pushButtonSingleStep_clicked();

private:
    Ui::EngineeringDialog *ui;
    int m_count;
};

#endif // ENGINEERINGDIALOG_H
