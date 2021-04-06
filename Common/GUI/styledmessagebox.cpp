/*
 * styledmessagebox.cpp
 *
 * styledMessageBox is a simple replacement for qt's default
 * QMessageBox in the Avinger style. This just allows us to
 * apply a custom style sheet to notification dialog boxes.
 *
 * Author: Chris White
 * Copyright (c) 2011-2018 Avinger, Inc.
 */
#include "styledmessagebox.h"
#include "ui_styledmessagebox.h"
#include <QStyle>
#include <QPushButton>
#include "logger.h"

styledMessageBox* styledMessageBox::m_instance {nullptr};
/*
 * Constructor
 */
styledMessageBox *styledMessageBox::instance()
{
    if(!m_instance){
        m_instance = new styledMessageBox(nullptr, "");
    }
    return m_instance;
}

styledMessageBox::styledMessageBox( QWidget *parent, QString msg ) :
    QDialog(parent),
    ui(new Ui::styledMessageBox)
{
    ui->setupUi( this );

    mask = new backgroundMask( parent );

    ui->messageText->setText( msg );

    setWindowFlags( Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() | Qt::FramelessWindowHint );

    ui->buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "&OK" ) );
    ui->buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&CANCEL" ) );
}

/*
 * Destructor
 */
styledMessageBox::~styledMessageBox()
{
    if( mask )
    {
        delete mask;
        mask = nullptr;
    }
    delete ui;
}

/*
 * setHasCancel()
 *
 * For some dialogs, we need a cancel button (if there is
 * a choice being made). For others, like errors, there
 * is no choice other than to dismiss.
 */
void styledMessageBox::setHasCancel( bool has, bool cancelIsDefault, bool hasOk )
{
    if( has )
    {
        if( hasOk )
        {
            ui->buttonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
        }
        else
        {
            ui->buttonBox->setStandardButtons( QDialogButtonBox::Cancel );
        }

        if( cancelIsDefault )
        {
            ui->buttonBox->button( QDialogButtonBox::Cancel )->setDefault( true );

            if( hasOk )
            {
                ui->buttonBox->button( QDialogButtonBox::Ok )->setAutoDefault( false );
            }
        }

        // This must be called again if the cancel button is added to the dialog
        ui->buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "&CANCEL" ) );
    }
    else
    {
        ui->buttonBox->setStandardButtons( QDialogButtonBox::Ok );
    }

    // option for message box without buttons
    if( !has && !hasOk )
    {
        ui->buttonBox->setStandardButtons( QDialogButtonBox::NoButton );
    }
}

/*
 * setTitle
 */
void styledMessageBox::setTitle( QString titleMsg )
{
    ui->titlebarTextLabel->setText( titleMsg.toUpper() );
}

/*
 * setInstructions
 */
void styledMessageBox::setInstructions(QString instructionsMsg )
{
    ui->instructionsTextLabel->setText( instructionsMsg.toUpper() );
}

/*
 * setText()
 *
 * Set the main message on the dialog box to the
 * passed in string.
 */
void styledMessageBox::setText( QString msg )
{
    ui->messageText->setText( msg );
}

/*
 * text()
 *
 * Return the currently set text on the dialog box.
 */
QString styledMessageBox::text( void )
{
    return ui->messageText->text();
}


/*
 * warning()
 *
 * Popup a simple warning box
 */
void styledMessageBox::warning( QString message )
{
    styledMessageBox msg;

    msg.setTitle( tr( "WARNING" ) );
    msg.setInstructions( tr( "" ) );
    msg.setText( message );
    msg.setHasCancel( false );
    msg.exec();

    LOG( WARNING, message.toLatin1() )
}

/*
 * info()
 *
 * Popup a purely informational message box
 */
void styledMessageBox::info( QString message )
{
    styledMessageBox msg;

    msg.setTitle( tr( "INFORMATION" ) );
    msg.setInstructions( tr( "" ) );
    msg.setText( message );
    msg.setHasCancel( false );
    msg.exec();

    LOG( INFO, message.toLatin1() )
}

/*
 * critical()
 *
 * Popup a critical failure message box
 */
void styledMessageBox::critical( QString message )
{
    styledMessageBox msg;

    msg.setTitle( tr( "ERROR" ) );
    msg.setInstructions( tr( "" ) );
    msg.setText( message );
    msg.setHasCancel( false );
    msg.exec();

    LOG( FATAL, message.toLatin1() )
}

void styledMessageBox::onShowMessageBox(const QString &msg, const QString &module, const QString &actionString)
{
    ui->titlebarTextLabel->setText(actionString);
    ui->messageText->setText(msg);
    ui->instructionsTextLabel->setText(module);
    setHasCancel( false );
    show();
}

void styledMessageBox::onHideMessageBox()
{
    hide();
}

void styledMessageBox::on_buttonBox_accepted()
{
    if (ui->titlebarTextLabel->text() != "Shutdown") {
        LOG(WARNING, ui->messageText->text());
        emit userAcknowledged();
    } else {
        QString s = QApplication::tr( ui->messageText->text().toLatin1() );
        LOG(FATAL, s.toLatin1().data())
    }
}
