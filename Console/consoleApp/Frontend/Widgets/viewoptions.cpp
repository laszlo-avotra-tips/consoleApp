/*
 * viewoptions.cpp
 *
 * Overlay widget to adjust view options and preferences
 * for the sector view, such as laser line
 * brightness, reticle brightness.
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#include "viewoptions.h"
#include "defaults.h"
#include "Utility/userSettings.h"
#include "logger.h"
#include "deviceSettings.h"

viewOptions::viewOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::viewOptions)
{
    ui->setupUi(this);

    userSettings &settings = userSettings::Instance();
    settings.loadSettings();

    ui->reticleSlider->setValue(                      settings.reticleBrightness() );
    ui->laserIndicatorSlider->setValue(               settings.laserIndicatorBrightness() );
    ui->invertColorsRadioButton->setChecked(          settings.invertOctColor() );
    ui->invertColorsRadioButton_no->setChecked(      !ui->invertColorsRadioButton->isChecked() );

    // Restore the saved slider value, but set the radio button off by default.
    ui->useNoiseReductionRadioButton->setChecked(     false );
    ui->useNoiseReductionRadioButton_no->setChecked( !ui->useNoiseReductionRadioButton->isChecked() );
    ui->noiseReductionSlider->setValue(               settings.noiseReduction() );
    ui->noiseReductionSlider->setEnabled(             false );

    ui->scanSyncLabel->setBuddy( ui->scanSyncSlider );

    // Force the control to be set so the initial state of the
    // view is saved in user settings and is logged.
    on_distalToProximalViewRadioButton_toggled( ui->distalToProximalViewRadioButton->isChecked() );

    // Colormap is Grayscale by default. Load from settings if we want this to be sticky.
    ui->grayscaleColorRadioButton->setChecked( true );
    ui->sepiaColorRadioButton->setChecked( !ui->grayscaleColorRadioButton->isChecked() );
}

viewOptions::~viewOptions()
{
    delete ui;
}

void viewOptions::disableButtons( void )
{
   ui->scanSyncSlider->setEnabled( false );
}

void viewOptions::enableButtons( void )
{
   ui->scanSyncSlider->setEnabled( true );
}

/*
 * on_laserIndicatorSlider_valueChanged()
 *
 * Adjust the pointer brightness based on the slider value,
 * inform the interested objects.
 */
void viewOptions::on_laserIndicatorSlider_valueChanged(int value)
{
    userSettings &settings = userSettings::Instance();
    settings.setLaserIndicatorBrightness( value );

    emit laserIndicatorBrightnessChanged( value );
}

/*
 * on_reticleSlider_valueChanged()
 *
 * Adjust the reticle brightness based on the slider value,
 * inform the interested objects.
 */
void viewOptions::on_reticleSlider_valueChanged(int value)
{
    userSettings &settings = userSettings::Instance();
    settings.setReticleBrightness( value );

    emit reticleBrightnessChanged( value );
}

/*
 * on_distalToProximalViewRadioButton_toggled
 *
 * Switch the point of view of the sector from "distal to proximal" (catheter pointed
 * down on the fluoro screen) to "proximal to distal" (catheter pointed up
 * on the fluoro screen) depending on the settings.
 */
void viewOptions::on_distalToProximalViewRadioButton_toggled(bool checked)
{
    userSettings &settings = userSettings::Instance();

    if( checked )
    {
        settings.setCatheterView( userSettings::DistalToProximal );
        LOG( INFO, "Catheter view: Down - distal to proximal" )
    }
    else
    {
        settings.setCatheterView( userSettings::ProximalToDistal );
        LOG( INFO, "Catheter view: Up - proximal to distal" )
    }

    emit updateCatheterView();
}

/*
 * on_scanSyncSlider_valueChanged
 *
 * Update the screen and settings for the lag angle.  Send out updates to the lag;
 * this works for both mouse and keyboard usage.
 */
void viewOptions::on_scanSyncSlider_valueChanged( int value )
{
    userSettings &settings = userSettings::Instance();
    settings.setLag( value );
    ui->scanSyncValueLabel->setText( QString( "%1" ).arg( value ) );
    LOG( INFO, QString( "Manual Scan Sync = %1" ).arg( value ))
    emit sendManualLagAngle( double(value) );
}

/*
 * handleNewLagAngle
 *
 * update the control if the lag angle has changed
 */
void viewOptions::handleNewLagAngle( double newLagAngle )
{
    ui->scanSyncSlider->setValue( int(newLagAngle) );
    ui->scanSyncValueLabel->setText( QString( "%1" ).arg( newLagAngle ) );
}

/*
 * resetLagAngle
 *
 * zero out the scan sync slider
 */
void viewOptions::setLagAngleToZero( void )
{
    handleNewLagAngle( 0 );
}

/*
 * handleDeviceChange
 *
 * enable/disable controls based on the device type
 */
void viewOptions::handleDeviceChange()
{
    deviceSettings &devSettings = deviceSettings::Instance();

//    if( devSettings.current()->isHighSpeed() )
    if(true)
    {
        ui->scanSyncSlider->setDisabled( true );
        ui->laserIndicatorSlider->setDisabled( true );
        ui->useNoiseReductionRadioButton->setEnabled( true );
        ui->useNoiseReductionRadioButton_no->setEnabled( true );
        ui->noiseReductionSlider->setEnabled( false ); // disable by default
    }
    else
    {
        ui->scanSyncSlider->setDisabled( false );
        ui->laserIndicatorSlider->setDisabled( false );
        ui->useNoiseReductionRadioButton->setEnabled( false );
        ui->useNoiseReductionRadioButton_no->setEnabled( false );
        ui->noiseReductionSlider->setEnabled( false );
    }
}

/*
 * on_useNoiseReductionRadioButton_toggled
 *
 * Turn on/off two-frame averaging. Applies to high speed devices only.
 */
void viewOptions::on_useNoiseReductionRadioButton_toggled( bool enable )
{
    LOG( INFO, QString( "Noise reduction enabled: %1" ).arg( enable ) )

    emit enableAveraging( enable );
    ui->noiseReductionSlider->setEnabled( enable );
}

/*
 * on_noiseReductionSlider_valueChanged
 *
 * Adjust the weights for averaging the previous and current frames
 */
void viewOptions::on_noiseReductionSlider_valueChanged( int position )
{
    userSettings &settings = userSettings::Instance();
    settings.setNoiseReductionVal( position );

    emit currFrameWeight_percentChanged(position);
}

/*
 * on_invertColorsRadioButton_toggled
 *
 * Signal how to draw the OCT image
 */
void viewOptions::on_invertColorsRadioButton_toggled(bool checked)
{
    LOG( INFO, QString( "Invert colors enabled: %1" ).arg( checked ) )
    userSettings &settings = userSettings::Instance();
    settings.setInvertOctColor( checked );

    emit enableInvertColors( checked );
}

/*
 * updateValues
 *
 * Sync the values after they are loaded from the System.ini file.
 */
void viewOptions::updateValues( void )
{
    emit reticleBrightnessChanged( ui->reticleSlider->value() );
    emit laserIndicatorBrightnessChanged( ui->laserIndicatorSlider->value() );
    emit enableInvertColors( ui->invertColorsRadioButton->isChecked() );
    emit enableAveraging( ui->useNoiseReductionRadioButton->isChecked() );
}

/* 
 * on_sepiaColorRadioButton_clicked 
 */
void viewOptions::on_sepiaColorRadioButton_clicked()
{
    // since it was clicked, it's new state becomes checked, now set the sibling off.
    ui->grayscaleColorRadioButton->setChecked( false );
    emit setColorModeSepia();
}

/* 
 * on_grayscaleColorRadioButton_clicked 
 */
void viewOptions::on_grayscaleColorRadioButton_clicked()
{
    // since it was clicked, it's new state becomes checked, now set the sibling off.
    ui->sepiaColorRadioButton->setChecked( false );
    emit setColorModeGray();
}
