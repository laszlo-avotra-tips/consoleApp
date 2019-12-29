/*
 * deviceselectwizardPage.cpp
 *
 * The deviceselectwizardpage is a customized qt wizard page allowing
 * the user to choose which device is to be used in the case. It presents
 * several presets with text/icon and a custom option.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */


#include "deviceSettings.h"
#include "deviceselectwizardpage.h"
#include "ui_deviceselectwizardpage.h"
#include "util.h"

deviceSelectWizardPage::deviceSelectWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::deviceSelectWizardPage)
{
    ui->setupUi(this);
    ui->listWidget->setDragEnabled( false );
}

void deviceSelectWizardPage::init( void )
{
    populateList();
    registerField("deviceList", ui->listWidget);
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)),   this, SIGNAL(completeChanged()));
    connect(ui->listWidget, SIGNAL(itemActivated(QListWidgetItem *)), this, SIGNAL(completeChanged()));
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );
}

deviceSelectWizardPage::~deviceSelectWizardPage()
{
    delete ui;
}

void deviceSelectWizardPage::changeEvent(QEvent *e)
{
    QWizardPage::changeEvent(e);
    switch (e->type()) 
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/*
 * populateList()
 *
 * Run through the device list and create a list item
 * for each device.
 */
void deviceSelectWizardPage::populateList(void)
{
    deviceSettings &devices = deviceSettings::Instance();

    // Only create the list if devices don't exist.
    if( devices.list().isEmpty() )
    {
        devices.init();
    }

    QList<device *>devList = devices.list();

    // size 16 font
    QFont deviceNameFont = QFont( "DinPRO-Medium", 16 );
    int fontHeight = QFontMetrics( deviceNameFont ).height();

    // 16 px spacing between items
    ui->listWidget->setSpacing( 16 );

    // 4 items per row and 2 rows can be displayed without scroll bar
    if( devList.size() <= 8 )
    {
        ui->listWidget->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
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
                                                   ui->listWidget,
                                                   0 );
        li->setFont( deviceNameFont );

        // allign text to top and center
        li->setTextAlignment( Qt::AlignTop );
        li->setTextAlignment( Qt::AlignHCenter );

        // add 20 px spacing on sides, add 2.2xfontHeight in vertical
        li->setSizeHint( QSize( d->getIcon().size() + QSize( 20, 2.2 * fontHeight ) ) );
    }
}

bool deviceSelectWizardPage::isComplete(void) const
{
    if( ui->listWidget->currentItem() != NULL )
    {
        return true;
    }
    return false;
}

void deviceSelectWizardPage::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->listWidget->setCurrentItem( item );
    wizard()->next();
}
