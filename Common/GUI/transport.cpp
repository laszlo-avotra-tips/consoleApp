/*
 * transport.cpp
 *
 * Playback controls for replaying recorded clips during a case.  The playback rate
 * values span 3x slower to 3x faster than real-time.
 *
 * Author: Dennis W. Jackson, Chris White
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */
#include <QDebug>
#include "transport.h"
#include "ui_transport.h"
#include <QTime>

/*
 * constructor
 */
transport::transport( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::transport )
{
    ui->setupUi( this );
    isPlaying = false;
    isClipSelected = false;

    connect( ui->timelineSlider, SIGNAL( sliderPressed() ), this, SLOT( seekingStart() ) );
    connect( ui->timelineSlider, SIGNAL( sliderReleased() ), this, SLOT( seekingEnd() ) );
}

/*
 * destructor
 */
transport::~transport()
{
    delete ui;
}

/*
 * changeEvent
 */
void transport::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
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
 * setPaused
 */
void transport::setPaused( bool paused )
{
    if ( paused )
    {
        isPlaying = false;
        ui->playButton->setIcon( QIcon( ":/octConsole/Frontend/Resources/play_button.png" ) );
    }
    else
    {
        isPlaying = true;
        ui->playButton->setIcon( QIcon( ":/octConsole/Frontend/Resources/pause_button.png" ) );
    }
}

/*
 * configureLayoutForTraining
 */
void transport::configureLayoutForTraining( void )
{
    ui->repeatCheckBox->setVisible( false );
    ui->clipLengthLabel->setAlignment( Qt::AlignHCenter );
    ui->currentClipLabel->hide();
}

/*
 * on_playButton_clicked
 */
void transport::on_playButton_clicked()
{
    // The control only works if a clip has been selected to play
    if( isClipSelected )
    {
        // Pause if playing; play if paused
        // While playing, display paused button for icon, vise versa while paused.
        if( !isPlaying )
        {
            emit play();
            ui->playButton->setIcon( QIcon( ":/octConsole/Frontend/Resources/pause_button.png" ) );
            isPlaying = true;
        }
        else
        {
            emit pause();
            ui->playButton->setIcon( QIcon( ":/octConsole/Frontend/Resources/play_button.png" ) );
            isPlaying = false;
        }
    }
}


/*
 * on_advanceButton_clicked
 */
void transport::on_advanceButton_clicked()
{
    emit advance();
}

/*
 * on_rewindButton_clicked
 */
void transport::on_rewindButton_clicked()
{
    emit rewind();
}

/*
 * handleClipName
 */
void transport::handleClipName( QString name )
{
    ui->currentClipLabel->setText( name );
    isClipSelected = true;
}

/*
 * handleClipLengthChanged
 */
void transport::handleClipLengthChanged( qint64 length_ms )
{
    // Add 500 ms and integer divide to round the result to the nearest second
    int length_s = int( ( length_ms + 500 ) / 1000 );
    QTime length_time = QTime(0, 0, 0, 0);
    length_time = length_time.addSecs( length_s );
    lengthString = length_time.toString( "h:mm:ss" );
    ui->clipLengthLabel->setText( QString( "0:00:00/" ) +lengthString );
    ui->timelineSlider->setMaximum( int(length_ms) );
}

/*
 * seekingStart
 */
void transport::seekingStart()
{
    if( isPlaying )
    {
       emit pause();
    }
}

/*
 * seekingEnd
 */
void transport::seekingEnd()
{
    emit seekRequest( ui->timelineSlider->sliderPosition() );
    if( isPlaying )
    {
        emit play();
    }
}

/*
 * updateClipPosition
 */
void transport::updateClipPosition( qint64 position )
{
    ui->timelineSlider->setValue( int(position ) );

    QTime pos_time = QTime( 0, 0, 0, 0);
    pos_time = pos_time.addMSecs( int(position) );
    QString posString = QString( "%1/%2 " ).arg( pos_time.toString( "h:mm:ss") ).arg( lengthString );
    ui->clipLengthLabel->setText( posString );
}

/*
 * isRepeatChecked
 */
bool transport::isRepeatChecked( void )
{
    return( ui->repeatCheckBox->isChecked() );
}
