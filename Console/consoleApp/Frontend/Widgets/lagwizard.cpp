/*
 * lagwizard.cpp
 *
 * The lagwizard is a customized qt wizard. It walks
 * the user through the lag correction process, taking
 * reference images and allowing alignment and verification.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#include "lagwizard.h"
#include "logger.h"
#include "ui_lagwizard.h"
#include "buildflags.h" /// XXX: R&D, cutter alignment

lagWizard::lagWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::lagWizard)
{
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::CancelButton << QWizard::FinishButton;
    setButtonLayout( layout );

    connect( this, SIGNAL(goToNext()), this, SLOT(next()) );
    ui->setupUi( this );
    angle = 0.0;
    leftImage  = new QImage( ui->leftImage->size(),  QImage::Format_ARGB32 );
    rightImage = new QImage( ui->rightImage->size(), QImage::Format_RGB32 );

    setWindowFlags( Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() | Qt::FramelessWindowHint );
    p = NULL;
    seenDirectionChange    = false;
    seenFirstRotation      = false;
    seenSecondRotation     = false;
    waitingForFirstCapture = true;

    setButtonText( QWizard::BackButton,   tr( "< &BACK" ) );
    setButtonText( QWizard::NextButton,   tr( "&NEXT >" ) );
    setButtonText( QWizard::CancelButton, tr( "&CANCEL" ) );
    setButtonText( QWizard::FinishButton, tr( "&FINISH" ) );
}

lagWizard::~lagWizard()
{
    delete ui;
    delete leftImage;
    delete rightImage;
    theScene->showMessage( tr( "" ) );
    seenDirectionChange    = false;
    seenFirstRotation      = false;
    seenSecondRotation     = false;
    waitingForFirstCapture = true;
}

void lagWizard::changeEvent(QEvent *e)
{
    QWizard::changeEvent( e );
    switch( e->type() )
    {
    case QEvent::LanguageChange:
        ui->retranslateUi( this );
        break;
    default:
        break;
    }
}

/*
 * setScene()
 *
 * Associated the wizards view with the scene so that
 * we can present small representations of the image.
 */
void lagWizard::setScene( QGraphicsScene *scene )
{
    // Reset angleInt in sectoritem.cpp. Now a full rotation is respective of the starting
    // point during the scan sync.
    emit resetIntegrationAngle();
    theScene = (liveScene *)scene;

    /*
     * Find the sector in the list of scene items
     */
    QGraphicsItem *hSector = scene->items().first();
    foreach( QGraphicsItem* item, scene->items() )
    {
        if( item->data( liveScene::SectorItemKey ) == "sector" )
        {
            hSector = item;
        }
    }

    ui->rotationView->setScene( scene );
    ui->rotationView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->rotationView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->rotationView->fitInView( QRectF( 0, 0, SectorWidth_px, SectorHeight_px ), Qt::KeepAspectRatio );

    ui->counterRotationView->setScene( scene );
    ui->counterRotationView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->counterRotationView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->counterRotationView->fitInView( QRectF( 0, 0, SectorWidth_px, SectorHeight_px ), Qt::KeepAspectRatio );
    theScene->showMessage( tr( "Scan Sync in progress. Please follow the directions of the operator." ) );
}

// SLOTS

/*
 * next()
 *
 * Advance to the next step
 */
void lagWizard::initializePage( int id )
{
    switch ( id ) 
    {
    case 1:
        p = new QPainter( leftImage );

        // remove text from the review image and take snapshot
        theScene->showMessage( "" );
        ui->rotationView->render( p );
        p->end();
        theScene->showMessage( tr( "Scan Sync in progress. Please follow the directions of the operator." ) );
        break;
    case 2:
        p = new QPainter( rightImage );

        // remove text from the review image and take snapshot
        theScene->showMessage( "" );
        ui->counterRotationView->render( p );
        p->end();
        theScene->showMessage( tr( "Scan Sync in progress. Please wait while the operator adjusts the Scan Sync value." ) );
        ui->leftImage->setPixmap( QPixmap::fromImage( *leftImage ) );
        ui->rightImage->setPixmap( QPixmap::fromImage( *rightImage ) );
        break;
    default:
        return;
        break;
    }
    if( p ) 
    {
        delete p;
        p = NULL;
    }
}

/*
 * on_angleSlider_valueChanged()
 *
 * Update the rotating image with the new angle
 * given by the slider.
 */
void lagWizard::on_angleSlider_valueChanged( int value )
{
    ui->leftImage->setAngle( value );
    angle = abs( value );
    ui->angleLabel->setText( QString( "%1" ).arg( abs( value ) ).append( QChar( 0x00B0 ) ) );
}

/*
 * handleDirectionChange()
 */
void lagWizard::handleDirectionChange( void )
{
    // If in States 1 or 2, restart the state machine. If in State 3, procede to State 4.
    seenFirstRotation   = false;
    seenDirectionChange = true;
    emit resetIntegrationAngle();

    LOG( DEBUG, "Scan Sync: handle direction changed" );
}

/*
 * handleFullRotation()
 *
 * This is a state machine used to automate Scan Sync. There are 6 states in the machine, and
 * an image is captured each time goToNext() is called. State 1 waits for a rotation, State 2 waits
 * for the second rotation, and then captures the first image and procedes to State 3. If a
 * direction change occurs during States 1 or 2, the state machine restarts in State 1.
 *
 * A direction change and three full rotations are required before capturing the second image
 * and completing the state machine. State 3 waits for a direction change, State 4 waits for
 * the first rotation, State 5 waits for the second rotation, and State 6 waits for the third
 * rotation and captures the second and final image. The state machine is complete after State 6
 * and procedes to the Scan Sync Alignment page of the lagwizard.
 */
void lagWizard::handleFullRotation( void )
{
    if( waitingForFirstCapture )
    {
        /* States 1 and 2. Waiting for two full rotations
         * before proceeding to the next state.
         */
        if( !seenFirstRotation )
        {
            seenFirstRotation = true;
            // Go to State 2

            LOG( INFO, "Scan Sync: First rotation detected ")
        }
        else
        {
            waitingForFirstCapture = false;
            seenFirstRotation = false;
            seenDirectionChange = false;
            emit goToNext();
            // Go to State 3

            LOG( INFO, "Scan Sync: First image captured" );
        }
    }
    else
    {
        /* States 4, 5, and 6. Wait for 3 full rotations
         * before capturing the second image and exiting
         * the state machine.
         */
        if( seenDirectionChange )
        {
            if( !seenFirstRotation )
            {
                // Rotation 1 of 3. Go to State 5
                seenFirstRotation = true;
                seenSecondRotation = false;

                LOG( INFO, "Scan Sync: First rotation after direction change detected" );
            }
            else
            {
                if( !seenSecondRotation )
                {
                    // Rotation 2 of 3. Go to State 6
                    seenSecondRotation = true;

                    LOG( INFO, "Scan Sync: Second rotation detected" );
                }
                else
                {
                    /* Rotation 3 of 3. Capture second image
                     * and exit the State Machine.
                     */
                    emit goToNext();

                    LOG( INFO, "Scan Sync: Second image captured" );
                }
            } // seenFirstRotation
        } // seenDirectionChange
    } // waitingForFirstCapture
}
