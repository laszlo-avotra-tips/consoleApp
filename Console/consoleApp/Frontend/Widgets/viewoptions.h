/*
 * viewoptions.h
 *
 * Overlay widget to adjust view options and preferences
 * for the sector and waterfall views, such as laser line
 * brightness, reticle brightness, and waterfall rate.
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#ifndef VIEWOPTIONS_H
#define VIEWOPTIONS_H

#include <QWidget>
#include <QSharedPointer>
#include "ui_viewoptions.h"
#include "buildflags.h"

namespace Ui {
    class viewOptions;
}

class viewOptions : public QWidget
{
    Q_OBJECT

public:
    viewOptions(QWidget *parent = nullptr);
    ~viewOptions();
    void disableButtons( void );
    void enableButtons( void );
    void updateValues( void );

signals:
    void reticleBrightnessChanged( int );
    void laserIndicatorBrightnessChanged( int );
    void updateCatheterView();
    void sendManualLagAngle( double );
    void enableAveraging( bool );
    void enableInvertColors( bool );
    void currFrameWeight_percentChanged(int);
    void displayWaterfall( bool );
    void setColorModeSepia();
    void setColorModeGray();

public slots:
    void handleNewLagAngle( double );
    void setLagAngleToZero();
    void handleDeviceChange();

private slots:
    void on_waterfallRateSlider_valueChanged(int value);
    void on_reticleSlider_valueChanged(int value);
    void on_laserIndicatorSlider_valueChanged(int value);
    void on_distalToProximalViewRadioButton_toggled(bool checked);
    void on_scanSyncSlider_valueChanged(int value);
    void on_useNoiseReductionRadioButton_toggled(bool checked);
    void on_noiseReductionSlider_valueChanged(int position);
    void on_showWaterfallRadioButton_toggled(bool checked);
    void on_invertColorsRadioButton_toggled(bool checked);
    void on_sepiaColorRadioButton_clicked();
    void on_grayscaleColorRadioButton_clicked();

private:
    Ui::viewOptions *ui;
};

#endif // VIEWOPTIONS_H
