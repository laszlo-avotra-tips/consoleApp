#include "dialogdeviceselect.h"
#include "ui_dialogdeviceselect.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "deviceSettings.h"
#include "util.h"
#include "logger.h"

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

    // size 16 font
//    QFont deviceNameFont = QFont( "DinPRO-Medium", 16 );
//    int fontHeight = QFontMetrics( deviceNameFont ).height();

//    // 16 px spacing between items
//    ui->listWidgetAtherectomy->setSpacing( 16 );

    // 4 items per row and 2 rows can be displayed without scroll bar
    if( devList.size() <= 8 )
    {
        ui->listWidgetAtherectomy->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    }

    device *d;
    foreach ( d, devList )
    {
        /*
         * This is not a leak.  QListWidget takes ownership of
         * the pointers.  It's a weird construct but it's the way
         * Qt adds items to the list widget.
         */
        QListWidgetItem *li = new QListWidgetItem( QIcon( QPixmap::fromImage( d->getIcon() ) ),
                                                   d->getDeviceName(),
                                                   ui->listWidgetAtherectomy,
                                                   0 );
//        li->setFont( deviceNameFont );

        // allign text to top and center
        li->setTextAlignment( Qt::AlignTop );
        li->setTextAlignment( Qt::AlignHCenter );

        // add 20 px spacing on sides, add 2.2xfontHeight in vertical
//        li->setSizeHint( QSize( d->getIcon().size() + QSize( 20, int(2.2 * fontHeight )) ) );
    }
}

void DialogDeviceSelect::on_pushButtonDone_clicked()
{
    //    WidgetContainer::instance()->gotoPage("mainPage");
    deviceSettings &dev = deviceSettings::Instance();
    dev.setCurrentDevice(0);


}

void DialogDeviceSelect::on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item)
{
    ui->listWidgetAtherectomy->setCurrentItem( item );
    LOG1(item->text());
}
