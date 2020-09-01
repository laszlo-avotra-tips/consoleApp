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
    populateList();
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );
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

    const bool isShowSpeed(dev->isAth());
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
        if(d->getDeviceName() == name.toString()){
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
        if(d->getDeviceName() == name.toString()){
            selection = i;
            break;
        }
        ++i;
    }
    dev.setCurrentDevice(selection);

    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}
