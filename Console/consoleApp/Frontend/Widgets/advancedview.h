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

#include <memory>

#include <QWidget>
#include <QTime>

#include "ui_advancedview.h"
#include "octFile.h"
#include "scanLine.h"
#include "buildflags.h"
#include "evoa.h"

namespace QtCharts{
class QChart;
}

class advancedView : public QWidget
{
    Q_OBJECT

public:
    advancedView(QWidget *parent = nullptr);
    ~advancedView();

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
    void checkSledStatus();

public slots:
    void addScanline( const OCTFile::OctData_t *pData );
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

private slots:
    void on_laserDiodeButton_clicked();
    void handleContrastChanged( int value );
    void handleBrightnessChanged( int value );
    void showRecordingFullCase( bool state );
    void getEvoaVoltage_v( double val );
    void on_tdcCheckBox_toggled( bool checked );
    void on_evoaSetDefaultButton_clicked();

    void on_getSledStatusButton_clicked();

private:
    void initLinePlot();

private:
    Ui::advancedViewClass ui;
    QTime timer;
    bool  diodeIsOn{false};
    int   lineCount{0};
    int   lastLagValue{0};
    EVOA  *evoa{nullptr};

    const int MaxSampleVal {4992};               // Defined by the length of the pre-resampled laser data
    const int MinADCVal {32768};                 // ATS card is +/- full range, start at 0V
    const int MaxADCVal {65535};                 // full range of 16-bit card
    const int MaxDepthVal {MaxALineLength - 1};  // Defined by the length of the FFT data after processing
    const int MaxdBVal_LowSpeed {65535};
    const int MaxdBVal_HighSpeed {255};
    const int NumEvoaChunks {5};                 // create a UI level gauge with 5 levels

    const QColor TitleColor { 0, 0, 0};
    const QFont AxisFont {"DinPRO-medium", 15};

    std::unique_ptr<QtCharts::QChart> m_linePlot;
};
