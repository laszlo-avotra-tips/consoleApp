#include "dialogdeviceselect.h"
#include "ui_dialogdeviceselect.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "deviceSettings.h"
#include "util.h"
#include "logger.h"
#include "mainwindow.h"
#include "Frontend/Screens/frontend.h"
#include <daqfactory.h>

DialogDeviceSelect::DialogDeviceSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDeviceSelect)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    ui->listWidgetAtherectomy->setDragEnabled(false);
    ui->listWidgetCto->setDragEnabled(false);
    init();
}

DialogDeviceSelect::~DialogDeviceSelect()
{
    delete ui;
}

void DialogDeviceSelect::init()
{
    populateList();
    connect(ui->listWidgetAtherectomy, SIGNAL(itemClicked(QListWidgetItem *)),   this, SIGNAL(completeChanged()));
    connect(ui->listWidgetAtherectomy, SIGNAL(itemActivated(QListWidgetItem *)), this, SIGNAL(completeChanged()));
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

}

bool DialogDeviceSelect::isComplete() const
{
    if( ui->listWidgetAtherectomy->currentItem() )
    {
        return true;
    }
    return false;

}

void DialogDeviceSelect::changeEvent(QEvent *e)
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

void DialogDeviceSelect::populateList()
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
        QListWidgetItem *li = new QListWidgetItem( QIcon( QPixmap::fromImage( d->getIcon() ) ),
                                                   d->getSplitDeviceName(),
                                                   ui->listWidgetAtherectomy,
                                                   0 );
        li->setTextAlignment( Qt::AlignRight );
        LOG2(d->getIcon().width(), d->getIcon().height());
    }
}

void DialogDeviceSelect::on_pushButtonDone_clicked()
{
    deviceSettings &dev = deviceSettings::Instance();
    int selection = ui->listWidgetAtherectomy->currentRow();
    dev.setCurrentDevice(selection);
    QWidget* widget = WidgetContainer::instance()->getPage("frontendPage");
    frontend* fw = dynamic_cast<frontend*>(widget);
    if(fw){
      fw->showFullScreen();
      fw->updateDeviceLabel();
      startDaq(fw);
    }
}

void DialogDeviceSelect::on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item)
{
    ui->listWidgetAtherectomy->setCurrentItem( item );
    LOG1(item->text());
}

void DialogDeviceSelect::startDaq(frontend *fe)
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

void DialogDeviceSelect::on_listWidgetAtherectomy_clicked(const QModelIndex &index)
{
    ui->frameDone->setStyleSheet("background-color: rgb(245,196,0); color: black");
}
