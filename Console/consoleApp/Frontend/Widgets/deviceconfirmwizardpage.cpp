/*
 * deviceConfirmWizardPage.cpp
 *
 * The deviceconfirmwizardpage is a customized qt wizard page allowing
 * the user to confirm their selection of imaging device.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#include "deviceconfirmwizardpage.h"
#include "deviceSettings.h"
#include "ui_deviceconfirmwizardpage.h"

/*
 * constructor
 */
deviceConfirmWizardPage::deviceConfirmWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::deviceConfirmWizardPage)
{
    ui->setupUi(this);
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );
}

/*
 * destructor
 */
deviceConfirmWizardPage::~deviceConfirmWizardPage()
{
    delete ui;
}

/*
 * changeEvent
 */
void deviceConfirmWizardPage::changeEvent(QEvent *e)
{
    QWizardPage::changeEvent(e);
    //    switch ( e->type() ) {
    //    case QEvent::LanguageChange:
    //        ui->retranslateUi( this );
    //        break;
    //    default:
    //        break;
    //    }
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi( this );
    }
}

/*
 * validatePage
 */
bool deviceConfirmWizardPage::validatePage()
{
    deviceSettings &dev = deviceSettings::Instance();
    dev.setCurrentDevice( field( "deviceList" ).toInt() );
    return true;
}

/*
 * initializePage()
 *
 * Called right before the page is displayed, giving
 * us a chance to gather up the previously selected
 * data from the "field" and paste it into the query
 * text.
 */
void deviceConfirmWizardPage::initializePage()
{
    int index = field( "deviceList" ).toInt();
    deviceSettings &dev = deviceSettings::Instance();
    ui->confirmationLabel->setText( tr( "You have selected <b>%1</b>." ).arg( dev.deviceAt( index )->getDeviceName() ) );
    ui->deviceLabel->setPixmap( QPixmap::fromImage( dev.deviceAt( index )->getIcon() ) );
    ui->disclaimerLabel->setText( dev.deviceAt( index )->getDisclaimerText() );

     // Don't require the user to click <FINISH> if there isn't a disclaimer, just force accpting the wizard.
    if( ui->disclaimerLabel->text().isEmpty() )
    {
        validatePage();
        wizard()->accept();
    }
}
