/*
 * advancedview.cpp
 *
 * Implements the screen which displays raw data in plot form for
 * engineering debugging.Also displays some daq related information,
 * such as the current voltage range, number of samples, etc.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#include <QSettings>
#include "advancedview.h"
#include "defaults.h"
#include "version.h"
#include "Utility/userSettings.h"
#include "logger.h"
#include "styledmessagebox.h"
#include "dspgpu.h"
#include "buildflags.h"
#include "util.h"
#include "windowmanager.h"
#include "logger.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>

QT_CHARTS_USE_NAMESPACE


/*
 * constructor
 */
advancedView::advancedView( QWidget *parent )
    : QWidget( parent )
{
    // line/sec counting variables
    lineCount    = 0;
    lastLagValue = 0;
    diodeIsOn    = false;
    timer.start();

    ui.setupUi(this);

    initLinePlot();

//    m_linePlot

//lcv
//    // Set up the raw data display
//    ui.rawDataPlot->init( MaxSampleVal );
//    QwtText rawBottomText( "Samples" );
//    rawBottomText.setColor( TitleColor );
//    ui.rawDataPlot->setAxisTitle( QwtPlot::xBottom, rawBottomText );
//    ui.rawDataPlot->setAxisFont( QwtPlot::xBottom, AxisFont );
//    ui.rawDataPlot->setAxisScale( QwtPlot::xBottom, 0, MaxSampleVal );

//    QwtText rawLeftText( "ADC Values" );
//    rawLeftText.setColor( TitleColor );
//    ui.rawDataPlot->setAxisTitle( QwtPlot::yLeft, rawLeftText );
//    ui.rawDataPlot->axisTitle( QwtPlot::yLeft ).setColor( TitleColor );
//    ui.rawDataPlot->setAxisFont( QwtPlot::yLeft, AxisFont );
//    ui.rawDataPlot->setAxisScale( QwtPlot::yLeft, MinADCVal, MaxADCVal );

//    // Set up the FFT display
//    ui.fftDataPlot->init( MaxDepthVal );
//    ui.fftDataPlot->setAxisScale( QwtPlot::xBottom, 0, MaxDepthVal );
//    QwtText fftBottomText( "Depth" );
//    ui.fftDataPlot->setStyleSheet("background: rgb( 60, 60, 60 );");
//    ui.rawDataPlot->setStyleSheet("background: rgb( 60, 60, 60 );");

//    fftBottomText.setColor( TitleColor );
//    ui.fftDataPlot->setAxisTitle( QwtPlot::xBottom, fftBottomText );
//    ui.fftDataPlot->axisTitle( QwtPlot::xBottom ).setColor( TitleColor );
//    ui.fftDataPlot->setAxisFont( QwtPlot::xBottom, AxisFont );

//    QwtText fftLeftText( "'dB'" );  // the data is in dB but we're not really displaying that
//    fftLeftText.setColor( TitleColor );
//    ui.fftDataPlot->setAxisTitle( QwtPlot::yLeft, fftLeftText );
//    ui.fftDataPlot->axisTitle( QwtPlot::yLeft ).setColor( TitleColor );
//    ui.fftDataPlot->setAxisFont( QwtPlot::yLeft, AxisFont );
//    ui.fftDataPlot->setAxisScale( QwtPlot::yLeft, 0, MaxdBVal_LowSpeed );
//    ui.fftDataPlot->enableLevels();

//    connect( ui.fftDataPlot, SIGNAL(updateBrightness(int)), this, SLOT(handleBrightnessChanged(int)) );
//    connect( ui.fftDataPlot, SIGNAL(updateContrast(int)), this, SLOT(handleContrastChanged(int)) );

    ui.versionLabel->setText( getSoftwareVersionNumber() );

    ui.tdcCheckBox->hide(); //lcv

    /*
     * NOTE: The internal name is EVOA, but the user-facing name will be "Imaging Strength".
     *       We will still continue to use "EVOA" in the Service Settings page.
     */
    evoa = new EVOA();

    connect( ui.evoaControlWidget, SIGNAL( valueChanged( double ) ),
             evoa,                 SLOT(   updateVoltage( double ) ) );
    connect( evoa,                 SIGNAL( valueChanged( double ) ),
             this,                 SLOT(   getEvoaVoltage_v( double ) ) );
    connect( evoa,                 SIGNAL( statusChanged( QString ) ),
             ui.evoaStatusVal,     SLOT(   setText( QString ) ) );

    ui.evoaBox->show();

    ui.evoaControlWidget->init( NumEvoaChunks, float(evoa->getCurrVoltage()), "Imaging Strength", evoa->getMinVal(), evoa->getMaxVal() );
    QString style = ui.evoaControlWidget->styleSheet();

    // give the level indicator a darker background
    style.append( "QProgressBar{ background-color: rgb( 51, 51, 51 ); }" );
    ui.evoaControlWidget->setStyleSheet( style );
    ui.evoaControlWidget->show();

    ui.swEncoder->hide();
}

/*
 * destructor
 */
advancedView::~advancedView()
{
}

/*
 * addScanLine()
 *
 * Given frame data from the daq, update the two plots (fft and raw)
 * with the new data. The actual plots are updated via timers to
 * maintain interactive performance, this call simply updates their
 * data cache.
 */
void advancedView::addScanline( const OCTFile::OctData_t *pData )
{
//    LOG1(pData)
//    if( pData->advancedViewIfftData )
//    {
//        ui.rawDataPlot->plotData( pData->advancedViewIfftData );
//    }

//    if( ( pData->advancedViewFftData ) )
//    {
//        ui.fftDataPlot->plotData( pData->advancedViewFftData );
//    }

    // Rough updates/second counter
    lineCount++;

    if( timer.elapsed() > 1000 )
    {
        ui.linesPerSecondLabel->setText( QString( "%1" ).arg( lineCount ) );
        lineCount = 0;
        timer.restart();

        // request an update to the UI to make sure all pieces of the widget
        // refresh periodically
        update();
    }
}

/*
 * on_brightnessSlider_valueChanged()
 *
 * Adjust the brightness (black-level) based on the new
 * position of the slider.
 */
void advancedView::handleBrightnessChanged(int value)
{
//lcv    ui.fftDataPlot->changeBrightness(value);
    emit brightnessChanged(value); // Let the DSP know.

    userSettings &settings = userSettings::Instance();
    settings.setBrightness( value );
}

void advancedView::showRecordingFullCase(bool state)
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

/*
 * on_contrastSlider_valueChanged
 *
 * Adjust the contrast (white-level) based on the new
 * position of the slider.
 */
void advancedView::handleContrastChanged(int value)
{
    //lcv    ui.fftDataPlot->changeContrast(value);
    emit contrastChanged(value); // Let the DSP know.

    userSettings &settings = userSettings::Instance();
    settings.setContrast( value );
}

/*
 * handleDaqLevel()
 *
 * Display the new DAQ range settings if signalled to do so.
 */
void advancedView::handleDaqLevel(QString level)
{
    ui.daqLevelLabel->setText( level );
}

/*
 * on_laserDiodeButton_clicked
 *
 * Handle the laser diode button.  The diode is turned on in main(); when the
 * UI is created the laser will be on initially. The state is updated on entry
 * to advanced view and after every button click.
 */
void advancedView::on_laserDiodeButton_clicked()
{
    // switch the state and send out the request
    if( diodeIsOn )
    {
        styledMessageBox msgLaserOff;
        msgLaserOff.setTitle( tr( "LASER" ) );
        msgLaserOff.setInstructions( tr( "" ) );
        msgLaserOff.setText( tr( "Would you like to turn off the laser? Imaging will stop." ) );
        msgLaserOff.setHasCancel( true );
        msgLaserOff.exec();

        // turn off if dialog is accepted
        if ( msgLaserOff.result() )
        {
            emit turnDiodeOff();
            LOG( INFO, "Laser Diode: User disabled" )
        }
    }
    else
    {
        emit turnDiodeOn();
        LOG( INFO, "Laser Diode: User enabled" )
    }

    // Update the indicator for the laser diode
    emit checkLaserDiodeStatus();
}

/*
 * handleLaserDiodeStatus
 *
 * Update the indicator when the status signal is received.  A progress bar is used in
 * place of an LED; Qt doesn't have a standard boolean indicator!
 */
void advancedView::handleLaserDiodeStatus( bool isOn )
{
    int value = 0;
    if( isOn )
    {
        value = 100;
    }
    ui.laserDiodeStatus->setValue( value );
    diodeIsOn = isOn;
}

/*
 * handleRawDataLengthChange
 */
void advancedView::handleRawDataLengthChange( int size )
{
    LOG1(size)
    // Adjust the axis and amount of data to copy to the chart
//lcv    ui.rawDataPlot->init( size );
//lcv    ui.rawDataPlot->setAxisScale( QwtPlot::xBottom, 0, size );
}

/*
 * updateDaqUpdatesPerSecond
 */
void advancedView::updateDaqUpdatesPerSecond( int value )
{
    ui.daqUpdatesPerSecondLabel->setText( QString( "%1" ).arg( value ) );
}

/*
 * getEvoaVoltage_v
 */
void advancedView::getEvoaVoltage_v( double val )
{
    QString text;

    // Show 2 decimals in the text string
    text = text.setNum( val, 'f', 2 );
    ui.evoaVoltageVal_v->setText( text );
}

/*
 * on_tdcCheckBox_toggled
 * R&D only
 */
void advancedView::on_tdcCheckBox_toggled(bool checked)
{
    emit tdcToggled( checked );
    ui.linesPerSecondLabel->setText( "" ); // force to zero so it updates on the next frame received
}

/*
 * handleDeviceChange
 */
void advancedView::handleDeviceChange()
{
    deviceSettings &devSettings = deviceSettings::Instance();

    QSettings *settings = new QSettings( SystemSettingsFile, QSettings::IniFormat );
    evoa->setVoltageToDefault();
    ui.evoaVoltageVal_v->setText( settings->value( EvoaDefaultSetting, EvoaDefault_v ).toString() );
    ui.evoaControlWidget->init( NumEvoaChunks, float(evoa->getCurrVoltage()), "Imaging Strength", evoa->getMinVal(), evoa->getMaxVal() );

    // Set the state of the UI depending on the device selected
    if( devSettings.current()->isHighSpeed() )
    {
//lcv
//        ui.fftDataPlot->clearData();

//        ui.fftDataPlot->setBrightnessLimits( BrightnessLevels_HighSpeed.minValue, BrightnessLevels_HighSpeed.maxValue );
//        ui.fftDataPlot->setContrastLimits( ContrastLevels_HighSpeed.minValue, ContrastLevels_HighSpeed.maxValue );

//        ui.fftDataPlot->setAxisScale( QwtPlot::yLeft, 0, MaxdBVal_HighSpeed );

//        ui.fftDataPlot->changeBrightness( BrightnessLevels_HighSpeed.defaultValue );
//        ui.fftDataPlot->changeContrast( ContrastLevels_HighSpeed.defaultValue );

    }
    else
    {
        ui.tdcCheckBox->hide(); //lcv
//lcv
//        ui.fftDataPlot->setBrightnessLimits( BrightnessLevels_LowSpeed.minValue, BrightnessLevels_LowSpeed.maxValue );
//        ui.fftDataPlot->setContrastLimits( ContrastLevels_LowSpeed.minValue, ContrastLevels_LowSpeed.maxValue );

//        ui.fftDataPlot->setAxisScale( QwtPlot::yLeft, 0, MaxdBVal_LowSpeed );

//        ui.fftDataPlot->changeBrightness( BrightnessLevels_LowSpeed.defaultValue );
//        ui.fftDataPlot->changeContrast( ContrastLevels_LowSpeed.defaultValue );
    }


    // Set the tolerance for selecting a line to 10% of the full y-axis range
//lcv    ui.fftDataPlot->setMousePressTolerance( ui.fftDataPlot->axisScaleDiv( QwtPlot::yLeft )->upperBound() * 0.10 );

//lcv    ui.fftDataPlot->enableDisplayControls();

    ui.evoaStatusVal->setText( EvoaStatusDefault );
}

/*
 * setReviewState
 *
 * The UI is in the Review state; brightness and contrast are disabled.
 */
void advancedView::setReviewState()
{
//lcv    ui.fftDataPlot->disableDisplayControls();
}

/*
 * setLiveState
 *
 * The UI is in the Live state; brightness and contrast are enabled.
 */
void advancedView::setLiveState()
{
//lcv    ui.fftDataPlot->enableDisplayControls();
}

/*
 * on_evoaSetDefaultButton_clicked()
 */
void advancedView::on_evoaSetDefaultButton_clicked()
{
    LOG( INFO, "EVOA return to default clicked" )
    evoa->setVoltageToDefault();
    ui.evoaControlWidget->setValue( evoa->getCurrVoltage() );
}


/*
 * on_getSledStatusButton_clicked
 */
void advancedView::on_getSledStatusButton_clicked()
{
    emit checkSledStatus();
}

void advancedView::initLinePlot()
{
    auto& rawFrame = ui.rawDataPlot;
    auto& fftFrame = ui.fftDataPlot;

//    LOG2(rawFrame->height(), rawFrame->width())
    LOG2(fftFrame->height(), fftFrame->width())

//    m_linePlot = std::make_unique<QChart>();

//![1]
    QSplineSeries *series = new QSplineSeries();
//![1]

//![2]
    QList<QPointF> valueList
    {
        {0,4},
        {200,6},
        {400,8},
        {600,4},
        {800,2},
        {1000,1},
    };
    series->append(valueList);
    QPen pen;
//    pen.setWidth(4);
//    pen.setColor("orange");
    series->setPen(pen);
//    series->append(0, 6);
//    series->append(2, 4);
//    series->append(3, 8);
//    series->append(7, 4);
//    series->append(10, 5);
//    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);
//![2]

//![3]
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTheme(QChart::ChartThemeDark);
    QBrush brush("yellow");
    chart->setTitle("Energy Spectrum");
    chart->setTitleBrush(brush);
    QFont font("courier new",14);
    font.setBold(true);

    chart->setTitleFont(font);
//![3]

//![4]
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
//![4]

    QVBoxLayout* vboxLayout = new QVBoxLayout(this);

    vboxLayout->addWidget(chartView);
    fftFrame->setLayout(vboxLayout);

    auto marg = chart->margins();
    LOG2(marg.top(), marg.bottom());

//    rawFrame->setLayout(vboxLayout);
}

/*
 * Update clocking status label
 */
void advancedView::displayClockingMode( int mode )
{
    switch( mode )
    {
      case 0:
          ui.sledModeValuelabel->setText( "Correction Off" );
          break;
      case 1:
          ui.sledModeValuelabel->setText( "Normal" );
          break;
      case 2:
          ui.sledModeValuelabel->setText( "Unknown" );
          break;
      case 3:
          ui.sledModeValuelabel->setText( "N/A" );
          break;
      default:
          ui.sledModeValuelabel->setText( "Unknown" );
    }
}

/*
 * displayFirmwareVersions
 */
void advancedView::displayFirmwareVersions( QByteArray vSled, QByteArray vSSB )
{
    ui.fwVersionVal->setText( QString( "%1 %2" ).arg( QString( vSled ) ).arg( QString( vSSB ) ) );
}

/*
 * attenuateLaser
 */
void advancedView::attenuateLaser( bool attenuate )
{
    evoa->pauseEvoa( attenuate );
    ui.evoaSetDefaultButton->setDisabled( attenuate );
    ui.evoaControlWidget->setDisabled( attenuate );

    if( attenuate )
    {
        ui.evoaControlWidget->setValue( double(evoa->getMinVal() ) );
    }
    else
    {
        ui.evoaControlWidget->setValue( evoa->getCurrVoltage() );
    }
}
