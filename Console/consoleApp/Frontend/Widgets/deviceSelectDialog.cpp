#include "deviceSelectDialog.h"
#include "ui_deviceSelectDialog.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "deviceSettings.h"
#include "util.h"
#include "logger.h"
#include "mainScreen.h"
#include "Frontend/Screens/frontend.h"
#include <daqfactory.h>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

DeviceSelectDialog::DeviceSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSelectDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    ui->listWidgetAtherectomy->setDragEnabled(false);
    ui->listWidgetCto->setDragEnabled(false);
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
//    connect(ui->listWidgetAtherectomy, SIGNAL(itemClicked(QListWidgetItem *)),   this, SIGNAL(completeChanged()));
//    connect(ui->listWidgetAtherectomy, SIGNAL(itemActivated(QListWidgetItem *)), this, SIGNAL(completeChanged()));
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

}

bool DeviceSelectDialog::isComplete() const
{
    if( ui->listWidgetAtherectomy->currentItem() )
    {
        return true;
    }
    return false;

}

void DeviceSelectDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
//    switch (e->type())
//    {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
    if( e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
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

    // 4 items per row and 2 rows can be displayed without scroll bar
    if( devList.size() <= 4 )
    {
        ui->listWidgetAtherectomy->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    }

    for ( device* d : devList )
    {
        /*
         * This is not a leak.  QListWidget takes ownership of
         * the pointers.  It's a weird construct but it's the way
         * Qt adds items to the list widget.
         */
       QListWidgetItem *li = new QListWidgetItem(
                   QIcon( QPixmap::fromImage( d->getIcon() ) ),
                   d->getSplitDeviceName(),
                   ui->listWidgetAtherectomy,
                   0 );
        li->setTextAlignment( Qt::AlignLeft );
    }
}

void DeviceSelectDialog::on_pushButtonDone_clicked()
{
    deviceSettings &dev = deviceSettings::Instance();
    int selection = ui->listWidgetAtherectomy->currentRow();
    dev.setCurrentDevice(selection);
    QWidget* widget = WidgetContainer::instance()->getScreen("l250Frontend");
    frontend* fw = dynamic_cast<frontend*>(widget);
    if(fw){
      fw->showFullScreen();
      fw->updateDeviceLabel();
      fw->showSpeed(false);
      startDaq(fw);
    }
}

void DeviceSelectDialog::on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item)
{
    ui->listWidgetAtherectomy->setCurrentItem( item );
    LOG1(item->text());
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

void DeviceSelectDialog::on_listWidgetAtherectomy_clicked(const QModelIndex &index)
{
    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonDone->setEnabled(true);
}
