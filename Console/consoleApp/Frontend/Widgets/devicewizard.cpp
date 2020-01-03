/*
 * devicewizard.cpp
 *
 * The devicewizard is a customized qt wizard. It walks
 * the user through the device selection, configuration, and verification
 * process prior to the start of a case.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */


#include "devicewizard.h"
#include "deviceSettings.h"
#include "ui_devicewizard.h"

/*
 * constructor
 */
deviceWizard::deviceWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::deviceWizard)
{
    ui->setupUi( this );
    ui->deviceConfirmPage->setFinalPage( true );
    setWindowFlags( Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() | Qt::FramelessWindowHint );

    setButtonText( QWizard::CancelButton, tr( "&CANCEL" ) );
    setButtonText( QWizard::BackButton,   tr( "< &BACK" ) );
    setButtonText( QWizard::NextButton,   tr( "&NEXT >" ) );
    setButtonText( QWizard::FinishButton, tr( "&FINISH" ) );
}

/*
 * destructor
 */
deviceWizard::~deviceWizard()
{
    delete ui;
}

/*
 * init
 */
void deviceWizard::init( void )
{
    // initialize the selection page
    ui->deviceSelectPage->init();
}

/*
 * changeEvent
 */
void deviceWizard::changeEvent( QEvent *e )
{
    QWizard::changeEvent(e);
//    switch (e->type())
//    {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
    }
}

/*
 * keyPressEvent
 *
 * Capture the ESC key to prevent it from closing the wizard if this is the first run through
 * device select. If not, that means a current device is selected, and allow the user to cancel
 * the wizard through the Escape key.
 */
void deviceWizard::keyPressEvent( QKeyEvent *e )
{
    deviceSettings &dev = deviceSettings::Instance();

    // Capture the ESC key and prevent it from closing the window if no current device exists.
    if( !dev.current() && e->key() == Qt::Key_Escape )
    {
        return;
    }
    else // with a device selected, allow the user to cancel the wizard.
    {
        // everything passes through to the parent and handled there
        QWizard::keyPressEvent( e );
    }
}

/*
 * closeEvent()
 *
 * Capture alt-F4, window close events.  Do not allow
 * the user to cancel this wizard.  Once they are in it, they
 * must select a device to leave it.
 */
void deviceWizard::closeEvent( QCloseEvent *e )
{
    e->ignore();
}
