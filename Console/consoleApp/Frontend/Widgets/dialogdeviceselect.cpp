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
                                                   formatDeviceName(d->getDeviceName()),
                                                   ui->listWidgetAtherectomy,
                                                   0 );
        li->setTextAlignment( Qt::AlignRight );
        LOG2(d->getIcon().width(), d->getIcon().height());
    }
}

QString DialogDeviceSelect::formatDeviceName(const QString &name)
{
    QString retVal;
    QStringList words = name.split(" ");
    if(words.size() == 3){
        retVal = QString("%1 %2\n%3").arg(words[0]).arg(words[1]).arg(words[2]);
    }
    return retVal;
}

void DialogDeviceSelect::on_pushButtonDone_clicked()
{
    //    WidgetContainer::instance()->gotoPage("mainPage");
    deviceSettings &dev = deviceSettings::Instance();
    int selection = ui->listWidgetAtherectomy->currentRow();
    dev.setCurrentDevice(selection);


}

void DialogDeviceSelect::on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item)
{
    ui->listWidgetAtherectomy->setCurrentItem( item );
    LOG1(item->text());
}
