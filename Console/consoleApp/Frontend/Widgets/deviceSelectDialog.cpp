#include "deviceSelectDialog.h"
#include "ui_deviceSelectDialog.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "deviceSettings.h"
#include "util.h"
#include "logger.h"
#include "mainScreen.h"
#include "deviceListModel.h"
#include "deviceDelegate.h"
#include "deviceDisplayModel.h"
#include "idaq.h"

#include <daqfactory.h>
#include <QImage>
#include <QIcon>
#include <QStringListModel>
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
    delete m_model;
    delete m_ctoModel;
}

void DeviceSelectDialog::initDialog()
{

    /*
     * Set opacity with animation
     */
    const int animationDuration_ms=1000;
    const QByteArray property{"opacity"};
    const float startValue{1.0f};
    const float endValue{0.9f};

    QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(this);
    QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, property);
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);


    setGraphicsEffect(showing_effect);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setDuration(animationDuration_ms);
    group->addAnimation(animation);
    group->start();

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    populateList();
}

void DeviceSelectDialog::populateList()
{
    deviceSettings &devices = deviceSettings::Instance();


    // Only create the list if devices don't exist.
    if( devices.list().isEmpty() )
    {
        devices.init();
    }
    LOG1(devices.list().size())
    if(devices.list().size() <= 3){
        ui->listViewAtherectomy->setStyleSheet("QFrame{border: 0px;border-right: 2px solid #ffffff;}");
    } else {
        ui->listViewAtherectomy->setStyleSheet("QFrame{border: 0px;}");
    }

    ui->listViewAtherectomy->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded );
    ui->listViewCto->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded );

    m_model = new DeviceListModel(this);
    m_ctoModel = new DeviceListModel(this);
    m_model->populate(false);
    m_ctoModel->populate(true);

    ui->listViewAtherectomy->setModel(m_model);
    ui->listViewCto->setModel(m_ctoModel);
    DeviceDelegate* delegate = new DeviceDelegate(this);
    ui->listViewAtherectomy->setItemDelegate(delegate);
    ui->listViewCto->setItemDelegate(delegate);
}

void DeviceSelectDialog::on_pushButtonDone_clicked()
{
    initializeSelectedDevice();

    deviceSettings &devices = deviceSettings::Instance();
    const auto& dev = devices.deviceAt(devices.getCurrentDevice());

    const bool isShowSpeed(!dev->isAth());
    QWidget* widget = WidgetContainer::instance()->getScreen("mainScreen");
    MainScreen* ms = dynamic_cast<MainScreen*>(widget);
    if(ms){
      ms->showFullScreen();
      ms->setDeviceLabel();
      ms->showSpeed(isShowSpeed);
      startDaq(ms);
    }
}

void DeviceSelectDialog::startDaq(MainScreen *ms)
{
    auto idaq = daqfactory::instance()->getdaq();

    if(!idaq){
        LOG( INFO, "Device not supported. OCT Console cancelled" )
        return;
    }
    if(idaq){
        if(idaq->getSignalSource()){
            connect( idaq->getSignalSource(), &IDAQ::updateSector, ms, &MainScreen::updateSector);
        }
        idaq->init();
        idaq->start();
    }
}

void DeviceSelectDialog::on_listViewAtherectomy_clicked(const QModelIndex &index)
{
    deviceSettings &dev = deviceSettings::Instance();

    QVariant name = index.data();
    LOG1(name.toString())

    int selection {0};
    int i{0};
    for(auto d : dev.list()){
        if(d->getSplitDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }

    m_model->setSelectedDeviceIndex(selection);

//    dev.setCurrentDevice(selection);

//    auto selectedDevice = dev.current();
//    auto speed = selectedDevice->getRevolutionsPerMin1();

//    MainScreen::setSpeed(speed);

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}

void DeviceSelectDialog::initializeSelectedDevice()
{
     deviceSettings &dev = deviceSettings::Instance();

     dev.setCurrentDevice(m_model->selectedDeviceIndex());
     auto selectedDevice = dev.current();
     auto speed = selectedDevice->getRevolutionsPerMin1();

     MainScreen::setSpeed(speed);
}



void DeviceSelectDialog::on_listViewCto_clicked(const QModelIndex &index)
{
    deviceSettings &dev = deviceSettings::Instance();

    QVariant name = index.data();
    LOG1(name.toString())

    int selection{0};
    int i{0};
    for(auto d : dev.list()){
        if(d->getSplitDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }
    m_model->setSelectedDeviceIndex(selection);

//    dev.setCurrentDevice(selection);

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}
