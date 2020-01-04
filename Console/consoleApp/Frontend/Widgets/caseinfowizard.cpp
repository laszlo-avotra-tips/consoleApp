/*
 * caseInfoWizard.cpp
 *
 * The caseinfowizard is a customized qt wizard allowing
 * data entry of session related data (patient, doctor, etc.)
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#include "caseinfowizard.h"
#include "Utility/captureListModel.h"
#include "Utility/userSettings.h"
#include "ui_caseinfowizard.h"
#include <QDir>

caseInfoWizard::caseInfoWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::caseInfoWizard)
{
    ui->setupUi(this);
    setWindowFlags( KeyboardWindowFlags );
    setWindowModality( Qt::WindowModal );

    setButtonText( QWizard::CancelButton, tr( "&CANCEL" ) );
    setButtonText( QWizard::FinishButton, tr( "&FINISH" ) );
}

caseInfoWizard::~caseInfoWizard()
{
    delete ui;
}

void caseInfoWizard::changeEvent(QEvent *e)
{
    QWizard::changeEvent(e);
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

void caseInfoWizard::init( SetupType type )
{
    ui->procdataPage->init( type );

    // Force the default case info values for initial setup.
    if( type == InitialCaseSetup )
    {
        ui->procdataPage->validatePage();
    }
}
