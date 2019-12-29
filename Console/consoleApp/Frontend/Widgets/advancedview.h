/*
 * advancedview.h
 *
 * Implements the screen which displays raw data in plot form for
 * engineering debugging.Also displays some daq related information,
 * such as the current voltage range, number of samples, etc.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger, Inc.
 */
#pragma once

#include <QWidget>
#include <QSharedPointer>
#include <QTime>
#include "ui_advancedview.h"
#include "octFile.h"
#include "scanLine.h"
#include "buildflags.h"
#include "evoa.h"

class advancedView : public QWidget
{
    Q_OBJECT

public:
    advancedView(QWidget *parent = 0);
    ~advancedView();

public slots:
    void addScanline( const OCTFile::FrameData_t *pData );
    void handleDaqLevel( QString );
    void handleLaserDiodeStatus( bool );
    void handleRawDataLengthChange( int size );
    void updateDaqUpdatesPerSecond( int value );
    void handleDeviceChange();
    void setReviewState();
    void setLiveState();
    void displayClockingMode( int );
    void displayFirmwareVersions( QByteArray, QByteArray );
    void attenuateLaser( bool );

signals:
    void brightnessChanged( int );
    void contrastChanged( int );
    void turnDiodeOn();
    void turnDiodeOff();
    void checkLaserDiodeStatus();
    void sendLagCorrectionEnabled( bool );
    void setEvoaVoltage_v( double );
    void setEvoaPowerLevel();
    void setEvoaToDefault( void );
    void tdcToggled( bool );
#if ENABLE_SLED_SUPPORT_BOARD_TESTING
    void checkSledStatus();
#endif
#if ENABLE_LOW_SPEED_DATA_SNAPSHOT
    void saveSignals();
#endif
#if CONSOLE_MANUFACTURING_RELEASE
    void enableOcelotSwEncoder( bool );
#endif

private:
    Ui::advancedViewClass ui;
    QTime timer;
    bool  diodeIsOn;
    int   lineCount;
    int   lastLagValue;
    EVOA  *evoa;
#if CONSOLE_MANUFACTURING_RELEASE
    QLabel *rawDataMaxLabel;
    int    rawDataLength;
#endif

private slots:
    void on_laserDiodeButton_clicked();
    void handleContrastChanged( int value );
    void handleBrightnessChanged( int value );
    void showRecordingFullCase( bool state )
    {
        if( state )
        {
            ui.fullCaseRecordingValueLabel->setText( "ENABLED" );
        }
        else
        {
            ui.fullCaseRecordingValueLabel->setText( "DISABLED" );
        }
    }
    void getEvoaVoltage_v( double val );
    void on_tdcCheckBox_toggled( bool checked );
    void on_evoaSetDefaultButton_clicked();

#if ENABLE_LOW_SPEED_DATA_SNAPSHOT
    void on_saveSignalsPushButton_clicked();
#endif
#if ENABLE_SLED_SUPPORT_BOARD_TESTING
    void on_getSledStatusButton_clicked();
#endif
};
