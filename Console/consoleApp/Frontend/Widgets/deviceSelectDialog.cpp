#include "deviceSelectDialog.h"
#include "ui_deviceSelectDialog.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "deviceSettings.h"
#include "util.h"
#include "logger.h"
#include "mainScreen.h"
#include "Frontend/Screens/frontend.h"
#include "deviceListModel.h"
#include "deviceDelegate.h"
#include "deviceDisplayModel.h"

#include <daqfactory.h>
#include <QImage>
#include <QIcon>
#include <QStringListModel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGestureEvent>
#include <QSwipeGesture>


DeviceSelectDialog::DeviceSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSelectDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    initDialog();
    grabGesture(Qt::SwipeGesture);
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
    deviceSettings &devices = deviceSettings::Instance();
    const auto& dev = devices.deviceAt(devices.getCurrentDevice());

    const bool isShowSpeed(!dev->isAth());
    QWidget* widget = WidgetContainer::instance()->getScreen("l250Frontend");
    frontend* fw = dynamic_cast<frontend*>(widget);
    if(fw){
      fw->showFullScreen();
      fw->updateDeviceLabel();
      fw->showSpeed(isShowSpeed);
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

    dev.setCurrentDevice(selection);

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
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
    dev.setCurrentDevice(selection);

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}

bool DeviceSelectDialog::event(QEvent *event)
{
    LOG1(event->type());
    if(event->type() == QEvent::Gesture){
        LOG1(event->type());

        QGestureEvent* ge = dynamic_cast<QGestureEvent*>(event);
        if(ge){
            return gestureEvent(ge);
        }
    }
    return QWidget::event(event);
}

//#include <QSwipeGesture>
bool DeviceSelectDialog::gestureEvent(QGestureEvent *ge)
{
    bool isHandled{false};
    LOG1(ge->type());
    if (QGesture *qg = ge->gesture(Qt::SwipeGesture))  {

        QSwipeGesture* sg = dynamic_cast<QSwipeGesture*>(qg);
        if(sg){
            const QSwipeGesture::SwipeDirection& sd =sg->verticalDirection();

            if(sd == QSwipeGesture::Up){
                LOG1("UP")
                isHandled = true;
            }
            if(sd == QSwipeGesture::Down){
                LOG1("DOWN")
                isHandled = true;
            }
        }
    }
    return isHandled;
}

