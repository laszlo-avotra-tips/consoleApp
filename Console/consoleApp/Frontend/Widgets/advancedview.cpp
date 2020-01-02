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

const int MaxSampleVal( 4992 );               // Defined by the length of the pre-resampled laser data
const int MinADCVal( 32768 );                 // ATS card is +/- full range, start at 0V
const int MaxADCVal( 65535 );                 // full range of 16-bit card
const int MaxDepthVal( MaxALineLength - 1 );  // Defined by the length of the FFT data after processing
const int MaxdBVal_LowSpeed( 65535 );
const int MaxdBVal_HighSpeed( 255 );
const int NumEvoaChunks( 5 );                 // create a UI level gauge with 5 levels

const QColor TitleColor( QColor( 0, 0, 0 ) );
const QFont AxisFont( "DinPRO-medium", 15 );

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
//#if CONSOLE_MANUFACTURING_RELEASE
//    ui.fftDataPlot->init( 512 );
//    ui.fftDataPlot->setAxisScale( QwtPlot::xBottom, 0, 3.3 );
//    ui.fftDataPlot->drawMmDepthLines();
//    QwtText fftBottomText( "Depth (mm)" );
//#else
//    ui.fftDataPlot->init( MaxDepthVal );
//    ui.fftDataPlot->setAxisScale( QwtPlot::xBottom, 0, MaxDepthVal );
//    QwtText fftBottomText( "Depth" );
//    ui.fftDataPlot->setStyleSheet("background: rgb( 60, 60, 60 );");
//    ui.rawDataPlot->setStyleSheet("background: rgb( 60, 60, 60 );");
//#endif
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

    connect( ui.fftDataPlot, SIGNAL(updateBrightness(int)), this, SLOT(handleBrightnessChanged(int)) );
    connect( ui.fftDataPlot, SIGNAL(updateContrast(int)), this, SLOT(handleContrastChanged(int)) );

    ui.versionLabel->setText( getSoftwareVersionNumber() );

#if !ENABLE_TOP_DEAD_CENTER_TOGGLE
    ui.tdcCheckBox->hide(); //lcv
#endif

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

    ui.evoaControlWidget->init( NumEvoaChunks, evoa->getCurrVoltage(), "Imaging Strength", evoa->getMinVal(), evoa->getMaxVal() );
    QString style = ui.evoaControlWidget->styleSheet();

    // give the level indicator a darker background
    style.append( "QProgressBar{ background-color: rgb( 51, 51, 51 ); }" );
    ui.evoaControlWidget->setStyleSheet( style );
    ui.evoaControlWidget->show();

#if !ENABLE_LOW_SPEED_DATA_SNAPSHOT
    ui.saveSignalsPushButton->hide();
#endif

#if CONSOLE_MANUFACTURING_RELEASE
    /*
     * Span from the normal starting point to the right edge of the monitor.
     * Don't change the height of the widget.
     */
    setGeometry( 0, 0, width() + 98, height() );
    ui.backgroundWidget->setGeometry( 0, 0, width(), height() );

    ui.highSpeedLabel->hide();
    ui.fftDataPlot->setStyleSheet("background: rgb( 60, 60, 60 );");
    ui.rawDataPlot->setStyleSheet("background: rgb( 60, 60, 60 );");

    ui.tdcCheckBox->hide();

    ui.fftDataPlot->setGeometry( ui.fftDataPlot->x(),
                                 ui.fftDataPlot->y(),
                                 this->width(),
                                 ui.fftDataPlot->height() );
    ui.rawDataPlot->setGeometry( ui.rawDataPlot->x(),
                                 ui.rawDataPlot->y(),
                                 this->width(),
                                 ui.rawDataPlot->height() );

    ui.gridLayoutWidget->show();
    ui.gridLayoutWidget->setGeometry( 10,
                                      ( this->height() - ui.gridLayoutWidget->height() - 10 ),
                                      ui.gridLayoutWidget->width(),
                                      ui.gridLayoutWidget->height() );

    ui.laserBox->show();
    ui.laserBox->setGeometry( ( this->width() - ui.laserBox->width() - 10 ),
                              ( this->height() - ui.laserBox->height() - 10 ),
                              ui.laserBox->width(),
                              ui.laserBox->height() );


    ui.evoaBox->show();
    ui.evoaSetDefaultButton->setGeometry( 10, 77, 161, 41 ); // move the button below the status box
    ui.evoaBox->setGeometry( ( this->width() - ui.evoaBox->width() - 10 ),
                             ( this->height() - ui.laserBox->height() - ui.evoaBox->height() - 20 ),
                             ui.evoaBox->width(),
                             ui.evoaBox->height() );

    WindowManager &wmgr = WindowManager::Instance();
    ui.wideDivideLabel->resize( wmgr.getTechnicianDisplayGeometry().width()/2, ui.wideDivideLabel->height() );
    ui.instructionsLabel->resize( wmgr.getTechnicianDisplayGeometry().width()/2, ui.instructionsLabel->height() );
    ui.titlebarLabel->resize( wmgr.getTechnicianDisplayGeometry().width()/2, ui.titlebarLabel->height() );

    ui.swEncoder->setGeometry( ( ui.gridLayoutWidget->width() + 10 ),
                               ( this->height() - 30 - 10 ),
                               100,
                               30 );
    connect( ui.swEncoder, SIGNAL( toggled(bool) ), this, SIGNAL( enableOcelotSwEncoder(bool) ) );

    rawDataMaxLabel = new QLabel( "", this );
    rawDataMaxLabel->show();
    rawDataMaxLabel->setGeometry( 330, 35, 300, 30 );
    rawDataLength = MaxSampleVal; // Use the default value until it is updated from a system INI file.
#else
    ui.swEncoder->hide();
#endif

#if !ENABLE_SLED_SUPPORT_BOARD_TESTING
    ui.getSledStatusButton->hide();
#endif
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
void advancedView::addScanline( const OCTFile::FrameData_t *pData )
{
//    if( pData->rawData )
//    {
//        ui.rawDataPlot->plotData( pData->rawData );
//    }

//    if( ( pData->fftData ) )
//    {
//        ui.fftDataPlot->plotData( pData->fftData );
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

#if CONSOLE_MANUFACTURING_RELEASE
        if( pData->rawData )
        {
            float max = 0;
            // find max val in raw data and display in a label
            for( int i = 0; i < rawDataLength; i++ )
            {
                if( max < ( (float)pData->rawData[ i ] / 1000 ) )
                {
                    max = ( (float)pData->rawData[ i ] / 1000 );
                }
            }
            // Format the number to display 55232 as 55.2 K, since this is more readable.
            rawDataMaxLabel->setText( QString( "Raw Data Max: %1 K" ).arg( QString::number( max, 'f', 1 ) ) );
        }
#endif
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
            LOG( INFO, "Laser Diode: User disabled" );
        }
    }
    else
    {
        emit turnDiodeOn();
        LOG( INFO, "Laser Diode: User enabled" );
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
    // Adjust the axis and amount of data to copy to the chart
//lcv    ui.rawDataPlot->init( size );
//lcv    ui.rawDataPlot->setAxisScale( QwtPlot::xBottom, 0, size );
#if CONSOLE_MANUFACTURING_RELEASE
    rawDataLength = size;
#endif
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
    ui.linesPerSecondLabel->setText( 0 ); // force to zero so it updates on the next frame received
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
    ui.evoaControlWidget->init( NumEvoaChunks, evoa->getCurrVoltage(), "Imaging Strength", evoa->getMinVal(), evoa->getMaxVal() );

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

#if ENABLE_TOP_DEAD_CENTER_TOGGLE
        ui.tdcCheckBox->show();
#endif
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

#if CONSOLE_MANUFACTURING_RELEASE
    int iMask_px = devSettings.current()->getInternalImagingMask_px();
    ui.fftDataPlot->drawInternalMaskLine( iMask_px );
    if( devSettings.current()->isHighSpeed() ) // only show for Low Speed
    {
        ui.swEncoder->hide();
    }
    else
    {
        ui.swEncoder->show();
        ui.swEncoder->setChecked( false );
    }
#endif
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
    LOG( INFO, "EVOA return to default clicked" );
    evoa->setVoltageToDefault();
    ui.evoaControlWidget->setValue( evoa->getCurrVoltage() );
}

#if ENABLE_LOW_SPEED_DATA_SNAPSHOT
/*
 * on_saveSignalsPushButton_clicked
 */
void advancedView::on_saveSignalsPushButton_clicked()
{
    emit saveSignals();
}
#endif

#if ENABLE_SLED_SUPPORT_BOARD_TESTING
/*
 * on_getSledStatusButton_clicked
 */
void advancedView::on_getSledStatusButton_clicked()
{
    emit checkSledStatus();
}
#endif

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
        ui.evoaControlWidget->setValue( evoa->getMinVal() );
    }
    else
    {
        ui.evoaControlWidget->setValue( evoa->getCurrVoltage() );
    }
}
