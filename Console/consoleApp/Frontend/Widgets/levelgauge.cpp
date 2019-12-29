/*
 * levelgauge.cpp
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2016-2018 Avinger, Inc.
 */
#include "levelgauge.h"
#include "ui_levelgauge.h"

const int MinPosition = 1; // don't let the gauge show empty, always show 1

/*
 * constructor
 */
levelGauge::levelGauge(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::levelGauge)
{
    ui->setupUi(this);
}

/*
 * destructor
 */
levelGauge::~levelGauge()
{
    delete ui;
}

/*
 * init
 *
 * This should be called at a device select. Calling this again will reset the position to
 * the default.
 */
void levelGauge::init( int desiredNumChunks,
                       double defaultVal,
                       QString title,
                       double minVal,
                       double maxVal )
{
    numChunks = desiredNumChunks;
    controlledObjMinVal = minVal;
    controlledObjMaxVal = maxVal;

    // calculate a scale factor
    scaleFactor = ( controlledObjMaxVal - controlledObjMinVal ) / (float)( numChunks - 1 );

    // map defaultVal to position
    // 0.5 required to round the value
    // +1 required to account for the min level value (always show 1, never go below).
    position = ( 1 + (int)( ( ( defaultVal - controlledObjMinVal ) / scaleFactor ) + 0.5 ) );

    ui->levelIndicator->setRange( 0, numChunks );
    ui->levelIndicator->setValue( position );

    ui->title->setText( title );

    // Make progress bar chunk size proportional to the desired range.
    float mg = 2.0f; // margin
    float bd = 2.0f; // border
    float sz = (float)ui->levelIndicator->width();
    float chunks = (float)desiredNumChunks;
    float chw = ( ( sz - bd - ( (2*mg) * (chunks-1.0f) ) ) / chunks ); // chunk width
    int i_chw = chw; // round down
    ui->levelIndicator->setStyleSheet( QString( "QProgressBar::chunk{ width: %1px; margin: %2px; border: %3px; }" ).arg( i_chw ).arg( mg ).arg( bd ) );
}

/*
 * on_leftButton_clicked
 *
 * If enabled, decrement the position, but don't go below the min.
 * Announce this position to connected objects.
 */
void levelGauge::on_leftButton_clicked()
{
    if( position > MinPosition )
    {
        position--;
        updatePosition();
    }
}

/*
 * on_rightButton_clicked
 *
 * If enabled, increment the position, but don't go above the max.
 * Annoucne this position to connected objects.
 */
void levelGauge::on_rightButton_clicked()
{
    if( position < numChunks )
    {
        position++;
        updatePosition();
    }
}

/*
 * updatePosition
 *
 * - Set the indicator to the new position.
 * - Translate the new position to the controlled object's value.
 * - Emit valueChanged.
 */
void levelGauge::updatePosition( void )
{
    ui->levelIndicator->setValue( position );
    double val = controlledObjMinVal + ( scaleFactor * (float)( position - 1 ) );
    emit valueChanged( val );
}

/*
 * setValue
 *
 * A public function that allows the accessor to set the indicator based on the
 * value in the controlled object's domain. This requires a translation to the
 * level indicator domain in terms of chunks.
 */
void levelGauge::setValue( double val )
{
    position = ( ( val - controlledObjMinVal ) / scaleFactor ) + 1;
    ui->levelIndicator->setValue( position );
}

/*
 * enableControls
 *
 * Make the title darkGray for disabled and lightGray for enabled.
 * Set the position to zero (empty) when disabled.
 */
void levelGauge::enableControls( bool enabled )
{
    this->setEnabled( enabled );

    if( enabled )
    {
        ui->title->setStyleSheet( "color: lightGray" );
        ui->levelIndicator->setValue( position ); // set at the current position
    }
    else
    {
        ui->title->setStyleSheet( "color: darkGray" );
        ui->levelIndicator->setValue( 0 );        // empty the tank
    }
}
