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
#include <signalmodel.h>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <algorithm>
#include <signalmanager.h>

QT_CHARTS_USE_NAMESPACE


/*
 * constructor
 */
advancedView::advancedView( QWidget *parent )
    : QWidget( parent )
{
    timer.start();

    ui.setupUi(this);

    initLinePlot();

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
void advancedView::addScanline()
{

    updatePlot();

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
    emit brightnessChanged(value); // Let the DSP know.

    SignalModel::instance()->setBlackLevel(value);
    userSettings &settings = userSettings::Instance();
    settings.setBrightness( value );
    qDebug() << __FUNCTION__ << "handleBrightnessChanged(int value)" << value;
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

    SignalModel::instance()->setWhiteLevel(value);
    userSettings &settings = userSettings::Instance();
    settings.setContrast( value );
    qDebug() << __FUNCTION__ << "handleContrastChanged(int value)" << value;
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
//    auto& rawFrame = ui.rawDataPlot;
    auto& fftFrame = ui.fftDataPlot;

//    LOG2(rawFrame->height(), rawFrame->width())
    LOG2(fftFrame->height(), fftFrame->width())

    m_linePlot = std::make_unique<QChart>();
    m_lineSeries = std::make_unique<QLineSeries>();
    m_whiteSeries = std::make_unique<QLineSeries>();
    m_blackSeries = std::make_unique<QLineSeries>();

    float whiteValue = 200.0f;
    float blackValue = 50.0f;

//![1]
    auto series = m_lineSeries.get();
    auto whiteSeries = m_whiteSeries.get();
    auto blackSeries = m_blackSeries.get();

//![1]

//![2]
    {
        QPen pen;
        pen.setWidth(1);
        QList<QPointF> values;
        for(size_t i = 0; i < 1024; ++i){
            if(i%3 == 0){
                values.push_back(QPointF(i, -10));
            }else{
                values.push_back(QPointF(i, 256));
            }
        }
        series->append(values);
        series->setPen(pen);
    }
//    {
//        QPen white;
//        white.setWidth(1);
//        white.setColor(QColor(255,255,255));
//        QList<QPointF> values;
//        for(size_t i = 0; i < 1024; ++i){
//                values.push_back(QPointF(i, whiteValue));
//        }
//        whiteSeries->append(values);
//        whiteSeries->setPen(white);
//    }
//    {
//        QPen black;
//        black.setColor(QColor(0,0,0));
//        black.setWidth(1);
//        QList<QPointF> values;
//        for(size_t i = 0; i < 1024; ++i){
//                values.push_back(QPointF(i, blackValue));
//        }
//        blackSeries->append(values);
//        blackSeries->setPen(black);
//    }


//![2]

//![3]
    auto chart = m_linePlot.get();//new QChart();

    chart->legend()->hide();
    chart->addSeries(series);
//    chart->addSeries(whiteSeries);
//    chart->addSeries(blackSeries);
    chart->createDefaultAxes();
    chart->setTheme(QChart::ChartThemeDark);
//    chart->setTheme(QChart::ChartThemeBlueIcy);
//    chart->setTheme(QChart::ChartThemeQt);
//    chart->setTheme(QChart::ChartThemeHighContrast);
//    chart->setTheme(QChart::ChartThemeBlueNcs);
    QBrush brush("white");
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

void advancedView::updatePlot()
{
    QMutexLocker guard(SignalManager::instance()->getMutex());

    const auto& values = SignalManager::instance()->getAdvancedViewFftPlotList();
    if(m_lineSeries->pointsVector().isEmpty()){
        m_lineSeries->append(values);
    } else {
        m_lineSeries->replace(values);
    }
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

void advancedView::handleAcqData()
{
//    static int count{0};
    auto* sModel = SignalModel::instance();
    auto* data = sModel->getAdvancedViewFrame();
    auto* sManager = SignalManager::instance();
    if(sManager->setAdvancedViewFftPlotList(data)){
//        qDebug() << __FUNCTION__ << ". " << data[0] << " " << data[1] << data[2] << data[3];
        updatePlot();
    }
}
