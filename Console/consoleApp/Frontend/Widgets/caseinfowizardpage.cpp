/*
 * caseInfoWizardPage.cpp
 *
 * The caseinfowizardpage is a customized qt wizard page allowing
 * data entry of session related data (patient, doctor, etc.)
 *
 * Handles the validation, saving, and setup of case information within
 * the wizard.
 *
 * Author: Chris White, Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#include "styledmessagebox.h"
#include <QString>
#include <QTime>
#include <QUuid>
#include "caseinfowizardpage.h"
#include "Utility/captureListModel.h"
#include "Utility/sessiondatabase.h"
#include "Utility/userSettings.h"
#include "ui_caseinfowizardpage.h"
#include "logger.h"
#include "util.h"
#include "version.h"
#include "buildflags.h"

/*
 * constructor
 */
caseInfoWizardPage::caseInfoWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::caseInfoWizardPage)
{
    ui->setupUi(this);
    ui->patientIdField->selectAll();

    ui->laserLabel->hide();

    caseSetupType     = caseInfoWizard::InitialCaseSetup;
}

/*
 * destructor
 */
caseInfoWizardPage::~caseInfoWizardPage()
{
    delete ui;
}

/*
 * init
 */
void caseInfoWizardPage::init( caseInfoWizard::SetupType type )
{
    autoCompleteSettings = new QSettings( SystemSettingsFile, QSettings::IniFormat );
    loadAutoComplete();

    // Set defaults
    const QString DefaultDoctor   = autoCompleteSettings->value( DefaultDoctorSetting, "" ).toString();
    const QString DefaultLocation = autoCompleteSettings->value( DefaultLocationSetting, "" ).toString();

    ui->doctorField->setText( DefaultDoctor );
    ui->locationField->setText( DefaultLocation );

    caseSetupType = type;

    // always a required field
    registerField( "PatientId*", ui->patientIdField );

    QRegExp rxPatientId( "^[-_:'A-Za-z0-9 ]*$" ); // Some text required
    QValidator *validatorPatientId = new QRegExpValidator( rxPatientId, this );
    ui->patientIdField->setValidator( validatorPatientId );

    registerField( "Doctor", ui->doctorField );
    registerField( "Location", ui->locationField );
    registerField( "Notes", ui->notesField );

    ui->versionField->setText( getSoftwareVersionNumber() );

    // hook the autocomplete lists to the appropriate fields
    docCompleter = new QCompleter( docList, this );
    docCompleter->setCaseSensitivity( Qt::CaseInsensitive );
    docCompleter->setCompletionMode( QCompleter::InlineCompletion );
    ui->doctorField->setCompleter( docCompleter );

    locationCompleter = new QCompleter( locationList, this );
    locationCompleter->setCaseSensitivity( Qt::CaseInsensitive );
    locationCompleter->setCompletionMode( QCompleter::InlineCompletion );
    ui->locationField->setCompleter( locationCompleter );

    // Updating case information. Load the current session's data
    // into the fields.
    if( caseSetupType == caseInfoWizard::UpdateCaseSetup )
    {
        caseInfo &info = caseInfo::Instance();
        ui->patientIdField->setText( info.getPatientID() );
        ui->doctorField->setText( info.getDoctor() );
        ui->locationField->setText( info.getLocation() );
        ui->notesField->setPlainText( info.getNotes() );
    }
}

/*
 * validatePage()
 *
 * Last chance to do anything after the page is finished.
 * Take this opportunity to setup the session and
 * the case info.
 */
bool caseInfoWizardPage::validatePage()
{
    /*
     * Populate the case info with the
     * fields given by the user.
     */
    caseInfo &info      = caseInfo::Instance();
    sessionDatabase db;

    // if patientId is empty, provide the current dateAndTime string.
    QString sPatientId = field( "PatientId" ).toString().trimmed();
    if( sPatientId.isEmpty() )
    {
        QDateTime now = QDateTime::currentDateTime();
        sPatientId = now.toString( QString( "yyyy-MM-dd hh:mm" ) );
    }

    // Set the case information
    info.setPatientID( sPatientId );
    info.setDoctor( field( "Doctor" ).toString().trimmed() );
    info.setLocation( field( "Location" ).toString().trimmed() );

    // field("Notes").toString() is null; access the text directly
    info.setNotes( ui->notesField->toPlainText() );

    /*
     * Create the session directory
     */
    if( caseSetupType == caseInfoWizard::InitialCaseSetup )
    {
        QDir dir;

        // set defaults
        info.setUtcOffset( 0 );

        /*
         * Create a unique case storage container.  By convention GUID's
         * have curly brackets around them; however, that causes mkisofs
         * to complain when exporting so they are removed for our use.
         */
        QString uuid = QUuid::createUuid().toString();
        uuid.remove( "{" ).remove( "}" );
        info.setCaseID( uuid );

        // Check the filesystem to see if this case id has been used. Extremely unlikely in this universe.
        if( dir.exists( info.getStorageDir() ) )
        {
            styledMessageBox::warning( tr( "Could not create a new case ID.  Contact Service at %1." ).arg( ServiceNumber ) );
            return false;
        }

        // Create the case session directories.  If any fail, report the error.
        if ( !( dir.mkdir( info.getStorageDir()  ) &&
                dir.mkdir( info.getClipsDir()    ) &&
                dir.mkdir( info.getCapturesDir() ) &&
                dir.mkdir( info.getFullCaseDir() ) ) )
        {
            styledMessageBox::critical(
                        tr( "File Failure:\nFailed to create session directory: " ) + info.getStorageDir() );
            return false;
        }
        else
        {
            // session directory structure was successfully created

            // determine how far off of UTC we are
            QDateTime now = QDateTime::currentDateTime();

            // check if we need to account for UTC being tomorrow relative to us
            int dayOffset = 0;
            if( now.date() < now.toUTC().date() )
            {
                dayOffset = 24;
            }

            info.setUtcOffset( now.time().hour() - ( now.toUTC().time().hour() + dayOffset ) );

            // Create and save the session information to the case database
            db.initDb();
            db.createSession();

            // save a cookie for HomeScreen to find
            updateSessionCookieFile( info.getCaseID() );

            LOG( INFO, QString( "Case ID: %1" ).arg( info.getCaseID() ) )
        }
    }
    else
    {
        // only updating case information
        db.updateSession();
    }

    // Update the autocomplete lists for future sessions
    saveAutoComplete();

    return true;
}


/*
 * changeEvent
 */
void caseInfoWizardPage::changeEvent(QEvent *e)
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
 * loadAutoComplete()
 *
 * Load the current autocomplete lists to the .ini file.
 */
void caseInfoWizardPage::loadAutoComplete( void )
{
    docList      = autoCompleteSettings->value( DoctorAutoCompleteSetting, "" ).toStringList();
    locationList = autoCompleteSettings->value( LocationAutoCompleteSetting, "" ).toStringList();
}

/*
 * saveAutoComplete()
 *
 * Save the current autocomplete lists to the .ini file.
 */
void caseInfoWizardPage::saveAutoComplete( void )
{
    caseInfo &info = caseInfo::Instance();

    // append entries to the list, sort it, and remove any duplicates
    if( !info.getDoctor().isEmpty() )
    {
        docList << info.getDoctor();
        docList.sort();
        docList.removeDuplicates();

        // on a new console, the list is always empty. Remove that entry after
        // a name is given
        if( docList.first() == "" )
        {
            docList.removeFirst();
        }

        autoCompleteSettings->setValue( DoctorAutoCompleteSetting, docList );
    }

    if( !info.getLocation().isEmpty() )
    {
        locationList << info.getLocation();
        locationList.sort();
        locationList.removeDuplicates();

        // on a new console, the list is always empty. Remove that entry after
        // a name is given
        if( locationList.first() == "" )
        {
            locationList.removeFirst();
        }

        autoCompleteSettings->setValue( LocationAutoCompleteSetting, locationList );
    }
}

/*
 * *_editingFinished
 *
 * Trim leading and trailing whitespace from fields for easier testing,
 * consistent input.
 */
void caseInfoWizardPage::on_patientIdField_editingFinished()
{
    ui->patientIdField->setText( field( "PatientId" ).toString().trimmed() );
}

void caseInfoWizardPage::on_doctorField_editingFinished()
{
    ui->doctorField->setText( field( "Doctor" ).toString().trimmed() );
}

void caseInfoWizardPage::on_locationField_editingFinished()
{
    ui->locationField->setText( field( "Location" ).toString().trimmed() );
}
