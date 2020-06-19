#include "deviceSelectDialog.h"
#include "ui_deviceSelectDialog.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "deviceSettings.h"
#include "util.h"
#include "logger.h"
#include "mainScreen.h"
#include "Frontend/Screens/frontend.h"
#include <daqfactory.h>
#include "consoleLabel.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>


DeviceSelectDialog::DeviceSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSelectDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    initDialog();
}

DeviceSelectDialog::~DeviceSelectDialog()
{
    delete ui;
}

void DeviceSelectDialog::initDialog()
{
    int duration_ms=1000;
    QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(this);
    QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, "opacity");
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    setGraphicsEffect(showing_effect);
    animation->setStartValue(0);
    animation->setEndValue(0.9);
    animation->setDuration(duration_ms);
    group->addAnimation(animation);
    group->start();
    populateList();
    populateList1();
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    connect(ui->labelAthText0, &ConsoleLabel::mousePressed, this, &DeviceSelectDialog::handleDevice0);
    connect(ui->labelAthText1, &ConsoleLabel::mousePressed, this, &DeviceSelectDialog::handleDevice1);
    connect(ui->labelAthText2, &ConsoleLabel::mousePressed, this, &DeviceSelectDialog::handleDevice2);
    connect(ui->labelText1, &ConsoleLabel::mousePressed, this, &DeviceSelectDialog::handleDevice3);

    connect(this, &DeviceSelectDialog::deviceSelected, this, &DeviceSelectDialog::handleDeviceSelected);

}

void DeviceSelectDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    if( e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void DeviceSelectDialog::populateList1()
{
    deviceSettings &devices = deviceSettings::Instance();

    // Only create the list if devices don't exist.
    if( devices.list().isEmpty() )
    {
        devices.init();
    }

    std::vector<QLabel*> deviceIconLabels{
        ui->labelIcon1, ui->labelIcon2, ui->labelIcon3
    };

    std::vector<QLabel*> deviceNames{
        ui->labelText1, ui->labelText2, ui->labelText3
    };

    QList<device *>deviceList = devices.list();

    int i = 3;
    for(auto* name : deviceNames){
        if(deviceList.size() > i){
            auto* device = deviceList[i++];
            if(device){
                name->setText(device->getDeviceName());
            }
        }
    }
    i = 3;
    for(auto* deviceIconLabel : deviceIconLabels){
        if(deviceList.size() > i){
            auto* device = deviceList[i++];
            if(device){
                const QImage& image = device->getIcon();
                deviceIconLabel->setText("");
                deviceIconLabel->setPixmap(QPixmap::fromImage( image ));
                deviceIconLabel->setMinimumSize(140,140);
                deviceIconLabel->setMaximumSize(140,140);
            }
        }
    }
}

void DeviceSelectDialog::populateList()
{
    deviceSettings &devices = deviceSettings::Instance();

    // Only create the list if devices don't exist.
    if( devices.list().isEmpty() )
    {
        devices.init();
    }

    QList<device *>devList = devices.list();

    std::vector<QLabel*> deviceIconLabels{
        ui->labelAthIcon0, ui->labelAthIcon1, ui->labelAthIcon2
    };

    std::vector<QLabel*> deviceNames{
        ui->labelAthText0, ui->labelAthText1, ui->labelAthText2
    };

    QList<device *>deviceList = devices.list();
    int i = 0;
    for(auto* name : deviceNames){
        if(deviceList.size() > i){
            auto* device = deviceList[i++];
            if(device){
                name->setText(device->getSplitDeviceName());
            }
        }
    }
    i = 0;
    for(auto* deviceIconLabel : deviceIconLabels){
        if(deviceList.size() > i){
            auto* device = deviceList[i++];
            if(device){
                const QImage& image = device->getIcon();
                deviceIconLabel->setText("");
                deviceIconLabel->setPixmap(QPixmap::fromImage( image ));
            }
        }
    }
}

void DeviceSelectDialog::on_pushButtonDone_clicked()
{
    QWidget* widget = WidgetContainer::instance()->getScreen("l250Frontend");
    frontend* fw = dynamic_cast<frontend*>(widget);
    if(fw){
      fw->showFullScreen();
      fw->updateDeviceLabel();
      fw->showSpeed(false);
      startDaq(fw);
    }
}


void DeviceSelectDialog::startDaq(frontend *fe)
{
    auto idaq = daqfactory::instance()->getdaq();

    if(!idaq){
        fe->abortStartUp();

        LOG( INFO, "Device not supported. OCT Console cancelled" )
    }
    fe->setIDAQ(idaq);
    LOG( INFO, "LASER: serial port control is DISABLED" )
    LOG( INFO, "SLED support board: serial port control is DISABLED" )

    fe->startDaq();
    auto& setting = deviceSettings::Instance();
    if(setting.getIsSimulation()){
        fe->startDataCapture();
    }
    fe->on_zoomSlider_valueChanged(100);
}

void DeviceSelectDialog::handleDevice0()
{
    highlight(ui->labelAthText0);
    emit deviceSelected(0);
}

void DeviceSelectDialog::handleDevice1()
{
    highlight(ui->labelAthText1);
    emit deviceSelected(1);
}

void DeviceSelectDialog::handleDevice2()
{
    highlight(ui->labelAthText2);
    emit deviceSelected(2);
}

void DeviceSelectDialog::handleDevice3()
{
    highlight(ui->labelText1);
    emit deviceSelected(3);
}

void DeviceSelectDialog::handleDeviceSelected(int did)
{
    deviceSettings &dev = deviceSettings::Instance();
    dev.setCurrentDevice(did);
    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}

void DeviceSelectDialog::removeHighlight()
{
    std::vector<QLabel*> deviceNames{
        ui->labelAthText0, ui->labelAthText1, ui->labelAthText2, ui->labelText1
    };
    for(auto* label : deviceNames) {
        label->setStyleSheet("");
    }
}

void DeviceSelectDialog::highlight(QLabel *label)
{
    removeHighlight();
    label->setStyleSheet("background-color:#646464");
}
