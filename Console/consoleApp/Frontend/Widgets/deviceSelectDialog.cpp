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
#include "displayManager.h"
#include "defaults.h"

#include <daqfactory.h>
#include <QImage>
#include <QIcon>
#include <QStringListModel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "Frontend/Utility/octFrameRecorder.h"
#include <QShowEvent>

DeviceSelectDialog::DeviceSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSelectDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    initDialog();

    const int xc = ControlScreenWidth / 2 - width() / 2;
    const int yc = ControlScreenHeight / 2 - height() / 2;
    move(xc,yc);

}

DeviceSelectDialog::~DeviceSelectDialog()
{
    delete ui;
    delete m_modelAtherectomy;
    delete m_modelCto;
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

void DeviceSelectDialog::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    if(se->type() == QEvent::Show){
        LOG1("show");
        DisplayManager::instance()->showOnTheSecondMonitor("disk");
        DisplayManager::instance()->setWindowTitle("DEVICE SELECTION IN PROCESS");
        WidgetContainer::instance()->setIsNewCase(true);
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
    LOG1(devices.list().size())
    if(devices.list().size() <= 3){
        ui->listViewAtherectomy->setStyleSheet("QFrame{border: 0px;border-right: 2px solid #ffffff;}");
        ui->listViewCto->setStyleSheet("QFrame{border: 0px;border-right: 2px solid #ffffff;}");
    } else {
        ui->listViewAtherectomy->setStyleSheet("QFrame{border: 0px;}");
        ui->listViewCto->setStyleSheet("QFrame{border: 0px;}");
    }

    ui->listViewAtherectomy->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded );
    ui->listViewCto->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded );

    m_modelAtherectomy = new DeviceListModel(this);
    m_modelCto = new DeviceListModel(this);
    m_modelAtherectomy->populate(false);
    m_modelCto->populate(true);

    ui->listViewAtherectomy->setModel(m_modelAtherectomy);
    ui->listViewCto->setModel(m_modelCto);

    m_delegate = new DeviceDelegate(this);
    ui->listViewAtherectomy->setItemDelegate(m_delegate);
    ui->listViewCto->setItemDelegate(m_delegate);

    ui->listViewAtherectomy->setViewMode(QListView::IconMode);
    ui->listViewCto->setViewMode(QListView::IconMode);
}

void DeviceSelectDialog::on_pushButtonDone_clicked()
{
    LOGUA;
    initializeSelectedDevice();

    deviceSettings &devices = deviceSettings::Instance();
    const auto& dev = devices.deviceAt(devices.getCurrentDevice());

    const bool isShowSpeed(!dev->isAth());

    QWidget* widget = WidgetContainer::instance()->getScreen("mainScreen");
    MainScreen* ms = dynamic_cast<MainScreen*>(widget);
    LOG2(widget,ms)
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
    if(idaq){
        if( idaq->startDaq() )
        {
            idaq->initDaq();
        } else {
            LOG1( "DAQ: Failed to start DAQ")
        }
    } else {
        LOG( INFO, "Device not supported. OCT Console cancelled" ) ;
    }
}

void DeviceSelectDialog::on_listViewAtherectomy_clicked(const QModelIndex &index)
{
    LOGUA;
    deviceSettings &dev = deviceSettings::Instance();

    QVariant name = index.data();
    LOG2(name.toString(), index.row())

    int selection {0};
    int i{0};
    for(auto d : dev.list()){
        if(d->getSplitDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }

    m_modelAtherectomy->setSelectedDeviceIndex(selection);
    auto* selectionModel = ui->listViewCto->selectionModel();
    selectionModel->clear();

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}

void DeviceSelectDialog::initializeSelectedDevice()
{
     deviceSettings &dev = deviceSettings::Instance();

     dev.setCurrentDevice(m_modelAtherectomy->selectedDeviceIndex());
}



void DeviceSelectDialog::on_listViewCto_clicked(const QModelIndex &index)
{
    LOGUA;
    deviceSettings &dev = deviceSettings::Instance();

    QVariant name = index.data();
    LOG2(name.toString(), index.row())

    int selection{0};
    int i{0};
    for(auto d : dev.list()){
        if(d->getSplitDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }

    m_modelAtherectomy->setSelectedDeviceIndex(selection);
    auto* selectionModel = ui->listViewAtherectomy->selectionModel();
    selectionModel->clear();

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}

void DeviceSelectDialog::on_listViewAtherectomy_pressed(const QModelIndex &index)
{
    LOGUA;
    deviceSettings &dev = deviceSettings::Instance();
    QVariant name = index.data();
    LOG2(name.toString(), index.row())

    int selection{0};
    int i{0};
    for(auto d : dev.list()){
        if(d->getSplitDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }
    auto* selectedDevice = dev.deviceAt(selection);
    dev.setSelectedIcon(selectedDevice->getIcon()[1]);

}

void DeviceSelectDialog::on_listViewCto_pressed(const QModelIndex &index)
{
    LOGUA;
    deviceSettings &dev = deviceSettings::Instance();
    QVariant name = index.data();
    LOG2(name.toString(), index.row())

    int selection{0};
    int i{0};
    for(auto d : dev.list()){
        if(d->getSplitDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }
    auto* selectedDevice = dev.deviceAt(selection);
    dev.setSelectedIcon(selectedDevice->getIcon()[1]);

}
