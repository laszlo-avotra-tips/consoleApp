/*
 * frontend.cpp
 *
 * The main window for the OCT application. This object is also
 * responsible for most of the GUI operations and delegations.
 * For example, it spawns off another window (the doctor screen),
 * handles initial GUI startup, controls, etc.
 *
 * Author: Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */
#include <QApplication>
#include <QScrollBar>
#include <QThread>
#include <QTime>
#include "frontend.h"
#include "logger.h"
#include "profiler.h"
#include "notificationwidget.h"
#include "Widgets/advancedview.h"
#include "Widgets/lagwizard.h"
#include "Widgets/devicewizard.h"
#include "deviceSettings.h"
#include "Utility/userSettings.h"
#include "Utility/captureListModel.h"
#include "Utility/clipListModel.h"
#include "Utility/sessiondatabase.h"
#include "sawFile.h"
#include "styledmessagebox.h"
#include "defaults.h"
#include "util.h"
#include "buildflags.h"
#include "Backend/depthsetting.h"
#include "sledsupport.h"
#include <QToolTip>
#include "Widgets/caseinfowizardpage.h"
#include "fileUtil.h"
#include "idaq.h"
#include "engineeringcontroller.h"
#include "signalmodel.h"
#include "forml300.h"
#include "mainScreen.h"
#include "Frontend/Utility/widgetcontainer.h"

// Configuration defines
#define HIGH_QUALITY_RENDERING 0


#if ENABLE_COLORMAP_OPTIONS
extern QImage sampleMap;
#endif

const int MouseCaptureInterval_ms      = 200;    // time to check mouse position
const int DaqWaitTimeout_ms            = 10000;  // XXX: really long. helps an issue with the High Speed DAQ not shutting down
                                                 //      quickly when changing devices. Needs more investigation. See #1149.
const int StorageSpaceCheckInterval_ms = 600000; // 10 min intervals
const int VariableDepthNumChunks = 5;

/*
 * Constructor
 */
frontend::frontend(QWidget *parent)
    : QWidget(parent), idaq(nullptr), m_ed(nullptr), m_ec(nullptr)
{
    m_scene    = nullptr;
    consumer = nullptr;

    appAborted        = false;
    isAnnotateOn        = false;

    isMeasureModeActive = false;

    isZoomModeOn        = false;
    isZooming           = false;
    zoomFactorText      = "";
    defaultZoomFactor   = 1.0;

#if ENABLE_SINGLE_STREAM_RECORDING
    isShuttingDown   = false;
#endif
    isImageCaptureLoaded  = false;

    isLoopLoaded     = false;
    isClipPlaying    = false;
    isRecordingClip  = false;
    disableCapturing = false;
    currClipNumber   = 0;
    currImgNumber    = 0;
    strClipNumber    = "";
    clipLength_ms    = 0;
    playbackClipName = "";
    clipCatheterView = "";

    preventFastRecordings       = false;
    isPhysicianPreviewDisplayed = false;

    ui.setupUi( this );

    ui.endCaseButton->setEnabled(false);

    // save the tip as defined in Designer so it can be restored as necessary
    defaultSceneToolTip = ui.liveGraphicsView->toolTip();

//    ui.loopMaskLabel->hide();

    // these are always hidden at start-up
//    ui.measureModePushButton->hide();
    ui.saveMeasurementButton->hide();


    ui.zoomLevelLabel->setBuddy( ui.zoomSlider );
    ui.zoomResetPushButton->hide();

    ui.liveViewPushButton->hide();

    // Hide things that only appear on demand.
    playbackControlsVisible( false );


    ui.recordingLabel->hide();

    /*
     * pre-create the advanced view
     */
    advView = new advancedView( this );
    advView->hide();

    /*
     * pre-create the View Options pane
     */
    viewOption = new viewOptions( this );
    viewOption->setGeometry( 3240 - viewOption->width() - 125,
                             0,
                             viewOption->width(),
                             viewOption->height() );

    viewOption->hide();

    // set the initial state
    userSettings &settings = userSettings::Instance();

    advView->setGeometry( 3240 - advView->width() - 125,
                          0,
                          advView->width(),
                          advView->height() );

    advView->hide();

    m_ec = new EngineeringController(this);

    m_scanWorker   = new ScanConversion();


    connect( ui.horizontalSliderBrigtness, SIGNAL(valueChanged(int)), advView, SLOT(handleBrightnessChanged(int)) );
    connect( ui.horizontalSliderContrast, SIGNAL(valueChanged(int)), advView, SLOT(handleContrastChanged(int)) );

    connect( advView, SIGNAL(brightnessChanged(int)), ui.horizontalSliderBrigtness , SLOT(setValue(int)) );
    connect( advView, SIGNAL(contrastChanged(int)),   ui.horizontalSliderContrast, SLOT(setValue(int)) );

//    connect( this,    SIGNAL(brightnessChange(int)), ui.displayControlsSlider, SLOT(setLowerPosition(int)) );
//    connect( this,    SIGNAL(contrastChange(int)),   ui.displayControlsSlider, SLOT(setUpperPosition(int)) );

    connect( advView, SIGNAL(brightnessChanged(int)),  this, SIGNAL(brightnessChange(int)) );
    connect( advView, SIGNAL(contrastChanged(int)),    this, SIGNAL(contrastChange(int)) );
    connect( advView, SIGNAL(turnDiodeOn()),           this, SIGNAL(forwardTurnDiodeOn()) );
    connect( advView, SIGNAL(turnDiodeOff()),          this, SIGNAL(forwardTurnDiodeOff()) );
    connect( advView, SIGNAL(checkLaserDiodeStatus()), this, SIGNAL(checkLaserDiodeStatus()) );
    connect( advView, SIGNAL(checkSledStatus()),       this, SIGNAL(checkSledStatus()) );

    connect( this, SIGNAL(forwardDaqLevel(QString)),                         advView, SLOT(handleDaqLevel(QString)) );
    connect( this, SIGNAL(forwardLaserDiodeStatus(bool)),                    advView, SLOT(handleLaserDiodeStatus(bool)) );
    connect( this, SIGNAL(announceClockingMode(int)),                        advView, SLOT(displayClockingMode(int)) );
    connect( this, SIGNAL(announceFirmwareVersions(QByteArray, QByteArray)), advView, SLOT(displayFirmwareVersions(QByteArray, QByteArray)) );

    connect( &session, SIGNAL(sendError(QString)),   this, SLOT(handleError(QString)) );
    connect( &session, SIGNAL(sendWarning(QString)), this, SLOT(handleWarning(QString)) );

    ui.horizontalSliderBrigtness->setValue(settings.brightness());
    ui.horizontalSliderContrast->setValue(settings.contrast());

    docWindow = nullptr;
    auxMon = nullptr;

    wmgr = &WindowManager::Instance();
    connect( wmgr, SIGNAL(monitorChangesDetected()), this, SLOT(handleScreenChanges()) );
    connect( wmgr, SIGNAL(badMonitorConfigDetected()), this, SLOT(handleBadMonitorConfig()) );
    wmgr->init();
    createDisplays();

    docWindow->ui.zoomFactorLabel->hide();

    /*
     * Create a new Aux monitor even if one is not connected.
     * Hard align the window to be beside the Tech + Physician Monitors.
     */

    handleStatusText( tr( "Initializing..." ) );

    /*
     * Set up all the items in the live view (sector, indicators, etc.)
     */
    setupScene();

    /*
     * Use a timer to sample the mouse location, make sure it
     * is kept within the technician window (this one).
     */
    mouseCaptureTimer.start( MouseCaptureInterval_ms );
    captureMouse( true );

    clockTimer.start( 1000 );

    /*
     * Hide the cursor after a period of inactivity.
     */
    hideMouseTimer.start( HideMouseDelay_ms );
    connect( &hideMouseTimer, SIGNAL(timeout()), this, SLOT(hideMouseTimerExpiry()) );

    // set up the time source for the on-screen clocks
    configureClock();

    // place Catheter Orientation label on monitors
    updateCatheterViewLabel();

    lagHandler = nullptr;

    // Set the focus on the Tech window so the menu keys are active
    QApplication::setActiveWindow( ui.centralWidget );

    // monitor events for the entire application (includes all child windows)
    if( qApp )
    {
        qApp->installEventFilter( this );
    }

    // Save the style sheet for restoring after using review mode
    origDeviceLabelStyleSheet = ui.deviceFieldLabel->styleSheet();
    origLiveQLabelStyleSheet = ui.label_live->styleSheet();

#if ENABLE_MEASUREMENT_PRECISION
    QLabel *measurementPrecisionLabel = new QLabel( "Measurement Precision Enabled\nNOT FOR HUMAN USE", this );
    measurementPrecisionLabel->setStyleSheet( "QLabel { font: 20px; color: red;}" );
    measurementPrecisionLabel->setGeometry( 10, 900, 991, 131 );
    measurementPrecisionLabel->show();
#endif
}

/*
 * Destructor
 */
frontend::~frontend()
{
    // If start-up is aborted the consumer thread never got started; don't try to stop it
    if( !appAborted )
    {
        // Shutdown the data consumer and wait for the thread to stop
        if( consumer )
        {
            consumer->stop();
            consumer->wait();
            delete consumer;
        }

        // shut down the daq; wait for it to stop
        if(idaq)
        {
            idaq->stop();
            idaq->wait( DaqWaitTimeout_ms );
            delete idaq;
        }

        // Free up all used memory. If we got here, these were all created.
        delete advView;
        delete viewOption;

        delete docWindow;
        docWindow = nullptr;  // docWindow is checked in the zoom pan handler

        delete m_scene;
        delete lagHandler;
        delete m_mainScreen;
    }
    else
    {
        // Shutdown session info.
        session.shutdown();

        // Delete the case directory since this is an aborted launch.
        caseInfo &info = caseInfo::Instance();
        fileUtil::deleteDir( info.getStorageDir() );
    }
}

/*
 * init()
 *
 * Connect various other objects to the frontend, set up the
 * initial session values and go.
 */
void frontend::init( void )
{
    TIME_THIS_SCOPE( frontend_init );
    lastDirCCW = true;			// make sure bidirectional devices start CCW (passive)

    LOG1(lastDirCCW);

    // Require case information before anything else happens
    caseWizard = std::make_unique<caseInfoWizard>(this);

    /*
     * Create the case info with default values (default doctor, default location, and
     * default PatientID. Don't call exec(), but call init() and validatePage();
     * accept() is required to save this info.
     */
    caseWizard->init( caseInfoWizard::InitialCaseSetup );
    caseWizard->accept();

    // Connect the error handler
    errorHandler &err = errorHandler::Instance();
    connect( &err, SIGNAL( warning( QString ) ), this, SLOT( handleWarning( QString ) ) );
    connect( &err, SIGNAL( failure( QString, bool ) ), this, SLOT( handleError( QString ) ) );

    // set-up the session and file handles
    session.init();

    // set up the capture and review widget
    ui.reviewWidget->init();

    // How we get the data from the DAQ to the Doc
    consumer = new DaqDataConsumer( m_scene,
                                    advView,
                                    session.getCurrentEventLog() );

    connect( consumer, &DaqDataConsumer::updateSector, this, &frontend::updateSector);

    connect( viewOption, SIGNAL( updateCatheterView() ), this,      SLOT( updateCatheterViewLabel() ) );
    connect( viewOption, SIGNAL( updateCatheterView() ), m_scene,     SLOT( clearSector() ) );

    connect( this, SIGNAL(sendLagAngle(double)), viewOption, SLOT(handleNewLagAngle(double)) );
    connect( viewOption, SIGNAL(sendManualLagAngle(double)), this, SLOT(handleManualLagAngle(double)) );

    connect( consumer, SIGNAL(updateAdvancedView()), advView, SLOT(addScanline()) );
    connect( this, SIGNAL(recordBackgroundData(bool)),  consumer, SLOT(recordBackgroundData(bool)) );
    connect( this, SIGNAL(tagEvent(QString)), consumer, SLOT(handleTagEvent(QString)) );

    connect( this,     SIGNAL( autoAdjustBrightnessAndContrast( void ) ), consumer, SLOT( handleAutoAdjustBrightnessAndContrast( void ) ) );
    connect( consumer, SIGNAL( updateBrightness( int ) ),                 advView,  SLOT( handleBrightnessChanged( int ) ) );
    connect( consumer, SIGNAL( updateContrast( int ) ),                   advView,  SLOT( handleContrastChanged( int ) ) );

    connect( m_scene, SIGNAL(sendCaptureTag(QString)), consumer, SLOT(handleTagEvent(QString)) );

    connect( m_scene, SIGNAL(sendStatusText(QString)), this, SLOT(handleStatusText(QString)) );

    connect( this, SIGNAL(disableMouseRotateSector()), m_scene, SLOT(handleDisableMouseRotateSector()) );
    connect( this, SIGNAL(enableMouseRotateSector()),  m_scene, SLOT(handleEnableMouseRotateSector()) );

    connect( &session, SIGNAL(sendSessionEvent(QString)), consumer, SLOT(handleTagEvent(QString)) );

    // clip recording signals
    connect( this,     SIGNAL(startClipRecording()),         consumer, SLOT(startClipRecording()) );
    connect( this,     SIGNAL(stopClipRecording()),          consumer, SLOT(stopClipRecording()) );
    connect( consumer, SIGNAL(sendVideoDuration( int )),     this,     SLOT(handleSendVideoDuration( int )) );

    connect( this,     SIGNAL(setClipFilename(QString)),     consumer, SLOT(setClipFile(QString)) );
    connect( consumer, SIGNAL(clipRecordingStopped()),       this,     SLOT(handleClipRecordingStopped()) );
    connect( this,     SIGNAL(captureClipImages(QString)),   m_scene,    SLOT(captureClip(QString)) );

    connect( consumer, SIGNAL(directionOfRotation(directionTracker::Direction_T)),
             m_scene,    SLOT(updateDirectionOfRotation(directionTracker::Direction_T)) );

    connect( consumer, SIGNAL(alwaysRecordingFullCase(bool)),     advView,  SLOT(showRecordingFullCase(bool)) );

    // setup the clip player and its signals
    connect( ui.transportWidget, SIGNAL( play() ),                 this, SLOT( handlePlayButton_clicked()) );
    connect( ui.transportWidget, SIGNAL( pause() ),                this, SLOT( handlePauseButton_clicked()) );
    connect( ui.transportWidget, SIGNAL( advance() ),             m_scene, SLOT( advancePlayback() ) );
    connect( ui.transportWidget, SIGNAL( rewind() ),              m_scene, SLOT( rewindPlayback()) );
    connect( ui.transportWidget, SIGNAL( seekRequest( qint64 ) ), m_scene, SLOT( seekWithinClip( qint64 ) ) );
    connect( m_scene, SIGNAL( clipLengthChanged( qint64 ) ),        ui.transportWidget, SLOT( handleClipLengthChanged( qint64 ) ) );

    connect( ui.reviewWidget, SIGNAL(sendLoopFilename(QString)), this,      SLOT(handleLoopLoaded(QString)) );
    connect( ui.reviewWidget, SIGNAL(sendStatusText(QString)),   this,      SLOT(handleStatusText(QString)) );
    connect( ui.reviewWidget, SIGNAL(displayingCapture()),       this,      SLOT(handleDisplayingCapture()) );
    connect( ui.reviewWidget, SIGNAL(sendDeviceName(QString)),   this,      SLOT(handleReviewDeviceName(QString)) );
    connect( ui.reviewWidget, SIGNAL(sendDeviceName(QString)),   docWindow, SLOT(setDeviceName(QString)) );

    connect( this, SIGNAL(setClipName(QString)),  ui.transportWidget, SLOT(handleClipName(QString)) );
    connect( this, SIGNAL(forcePauseButtonOff()), ui.transportWidget, SLOT(handleForcePauseButtonOff()) );

    connect( m_scene, SIGNAL(updateCaptureCount()),     ui.reviewWidget, SLOT(updateCaptureCount()) );
    connect( m_scene, SIGNAL(updateClipCount()),        ui.reviewWidget, SLOT(updateClipCount()) );
    connect( m_scene, SIGNAL( sendWarning( QString ) ),            this, SLOT( handleWarning( QString ) ) );
    connect( m_scene, SIGNAL( sendError( QString ) ),              this, SLOT( handleError( QString ) ) );

    connect( m_scene, SIGNAL( videoTick(qint64) ),       ui.transportWidget, SLOT( updateClipPosition( qint64 ) ) );
    connect( m_scene, SIGNAL( endOfFile() ),                           this, SLOT( handleEndOfFile() ) );
    connect( m_scene, SIGNAL( reviewImageDismissed( ) ),               this, SLOT( hideLiveViewButton() ) );

    clipListModel &clipList = clipListModel::Instance();
    connect( &clipList, SIGNAL( warning( QString ) ), this, SLOT( handleWarning( QString ) ) );

    connect( ui.liveGraphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(handleTechViewHorizontalPan(int)) );
    connect( ui.liveGraphicsView->verticalScrollBar(),   SIGNAL(valueChanged(int)), this, SLOT(handleTechViewVerticalPan(int)) );

    // Connect the messages for device select
    deviceSettings &dev = deviceSettings::Instance();
    connect( &dev, SIGNAL( sendFailure(QString) ), this, SLOT( handleError(QString) ) );
    depthSetting &depthManager = depthSetting::Instance();
    connect( &dev, SIGNAL( deviceChanged() ), &depthManager, SLOT( handleDeviceChange() ) );

    // connect the level gauge UI element with the depthSettings singleton object
    connect( ui.imagingDepthWidget, SIGNAL( valueChanged(double) ), &depthManager, SLOT( updateImagingDepth(double) ) );
    connect( m_formL300, SIGNAL( depthChanged(double) ), &depthManager, SLOT( updateImagingDepth(double) ) );
    ui.imagingDepthWidget->init( 5, 3, "DEPTH", 1, 5 ); // dummy settings that will be overwritten at device selection
    ui.imagingDepthWidget->setEnabled( true );

    // start this session
    session.start();

    // start monitoring available storage space
    storageSpaceTimer.start( StorageSpaceCheckInterval_ms );
    connect( &storageSpaceTimer, SIGNAL(timeout()), this, SLOT(storageSpaceTimerExpiry()) );

    connect( &preventFastRecordingsTimer, SIGNAL(timeout()), this, SLOT(reenableRecordLoopButtonExpiry()) );
}

/*
 * setupCase
 *
 * Launch the Case Wizard and get the case configuration from the user
 */
int frontend::setupCase( bool isInitialSetup )
{
    if( isInitialSetup )
    {
        // Launch the device selection wizard
//        return on_deviceSelectButton_clicked();
        return -1;
    }
    else // Launched from the case details button.
    {
        // Require case information before anything else happens
        caseInfoWizard *caseWizardLocal = new caseInfoWizard( this );

        // reload data for updating
        caseWizardLocal->init( caseInfoWizard::UpdateCaseSetup );

        // Force the wizard to the center of the primary monitor
        int x = ( wmgr->getTechnicianDisplayGeometry().width() - caseWizardLocal->width() ) / 2;
        int y = ( wmgr->getTechnicianDisplayGeometry().height() - caseWizardLocal->width() ) / 2;
        caseWizardLocal->setGeometry( x, y, caseWizardLocal->width(), caseWizardLocal->height() );

        // Get the case information.
        int result = caseWizardLocal->exec();
        delete caseWizardLocal;
        return result;
    }
}

void frontend::updateDeviceLabel()
{
    if(m_mainScreen){
        m_mainScreen->setDeviceLabel();
    }
}

/*
 * disableStorage
 *
 * Disable storage operations. Used during low space
 * conditions, etc.
 */
void frontend::disableStorage( bool disable )
{
    disableCapturing = disable;

    if( disable )
    {
        disableCaptureButtons();

        // disable full case recording if it was turned enabled
        consumer->disableFullCaseRecording();

        // inform the user
        styledMessageBox::warning( tr( "Captures are disabled due to low storage space.\nContact Service at %1." ).arg( ServiceNumber ) );

        LOG( WARNING, "Low disk space. Captures and recording disabled." )
    }
}

/*
 * setupScene
 *
 * Create the QGraphicsScene which contains the essential rendering items
 * used for the OCT images (sector, etc.)
 */
void frontend::setupScene( void )
{
    deviceSettings &dev = deviceSettings::Instance();

    m_scene = new liveScene( this );
//    m_formL300 = new FormL300( this );
//    m_formL300->setScene(m_scene);
    m_mainScreen = new MainScreen(this);
    m_mainScreen->setScene(m_scene);
//    m_mainWindow->showFullScreen();

    connect(m_mainScreen, &MainScreen::captureImage, this, &frontend::on_captureImageButton_clicked);
    connect(m_mainScreen, &MainScreen::measureImage, this, &frontend::setMeasurementMode);

    connect( &dev, SIGNAL(deviceChanged()), m_scene,      SLOT(handleDeviceChange()) );
    connect( &dev, SIGNAL(deviceChanged()), this,       SLOT(handleDeviceChange()) );
    connect( &dev, SIGNAL(deviceChanged()), advView,    SLOT(handleDeviceChange()) );
    connect( &dev, SIGNAL(deviceChanged()), viewOption, SLOT(handleDeviceChange()) );
//    connect( &dev, SIGNAL(deviceChanged()), ui.displayControlsSlider, SLOT(updateBrightnessContrastLimits()) );

    connect( m_scene, SIGNAL(showCurrentDeviceLabel()),    this, SLOT(handleShowCurrentDeviceLabel()) );

    connect( ui.reviewWidget, SIGNAL(showCapture(const QImage &)),
             m_scene,           SLOT(showReview( const QImage & )) );

    connect( ui.reviewWidget, SIGNAL(initCaptureWidget()),
             this,           SLOT(hideDecoration()) );

    connect( this,	SIGNAL(setDoPaint()),		m_scene, SLOT(setDoPaint()) );

    /*
     * Set pixelsPerMm conversion and zoom factor at the time of image capture for measurement calibration.
     * Use the pixelsPerMm value to disable the measurement feature.
     */
    connect( ui.reviewWidget, SIGNAL(sendReviewImageCalibrationFactors(int,float)),
             m_scene,           SLOT(setCalibrationScale(int,float)) );
    connect( ui.reviewWidget, SIGNAL(sendReviewImageCalibrationFactors(int,float)),
             this,            SLOT(enableDisableMeasurementForCapture(int)) );

    connect( ui.reviewWidget, SIGNAL(currentCaptureChanged(QModelIndex)),
             docWindow,       SLOT(updatePreview(QModelIndex)) );

    connect( viewOption, SIGNAL(reticleBrightnessChanged(int)),
             m_scene,      SLOT(handleReticleBrightnessChanged(int)) );

    connect( viewOption, SIGNAL(laserIndicatorBrightnessChanged(int)),
             m_scene,      SLOT(handleLaserIndicatorBrightnessChanged(int)) );

    connect( m_scene, SIGNAL(sendFileToKey(QString)), &session, SLOT(handleFileToKey(QString)) );

    // Auto fill the background with black
    m_scene->setBackgroundBrush( QColor( 0,0,0 ) );

    // Associate the views with the m_scene
    ui.liveGraphicsView->setMatrix( QMatrix() );

    docWindow->setScene( m_scene );
    auxMon->setScene( m_scene );

    // save the original transform matrix for this view to use with the zoom feature
    techViewMatrix = ui.liveGraphicsView->matrix();
    docViewMatrix  = docWindow->ui.liveGraphicsView->matrix();
    auxViewMatrix  = auxMon->getMatrix();

    // High quality scaling algorithms
#if HIGH_QUALITY_RENDERING
    ui.liveGraphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
#endif
}

/*
 * confirmExit()
 *
 * Ask the user for confirmation that before shutting down the application
 */
bool frontend::confirmExit( void )
{
    styledMessageBox msgBox( this );

    msgBox.setTitle( tr( "All data is saved." ) );
    msgBox.setInstructions( tr( "Do you want to end the case?" ) );
    msgBox.setHasCancel( true, true );
    msgBox.raise();
    int ret = msgBox.exec();

    return( ret == QDialog::Accepted );
}

// Signal handlers

/*
 * on_endCaseButton_clicked()
 *
 * Quit the application
 */
void frontend::on_endCaseButton_clicked()
{
    LOG( INFO, "End case button clicked" )
    if( confirmExit() )
    {
        LOG( INFO, "End case confirmation accepted" )
        stopDataCapture();
        emit recordBackgroundData( false );

        QApplication::quit();
    }
    else
    {
        LOG( INFO, "End case confirmation rejected" )
    }
}

/*
 * closeEvent()
 *
 * Capture alt-F4, window close events
 */
void frontend::closeEvent( QCloseEvent * /*event*/ )
{
    /*
     * go through a common exit point
     */
    on_endCaseButton_clicked();
}

/*
 * shutdownCleanup()
 *
 * This slot is signalled by the main application when it is about to
 * shutdown.  It performs any clean up that needs done and waits for a
 * short time before continuing the shutdown to allow any messages in flight
 * to land.
 */
void frontend::shutdownCleanup()
{
#if ENABLE_SINGLE_STREAM_RECORDING
    isShuttingDown = true;
#endif

    // close out any ongoing clip recording
    if( isRecordingClip )
    {
        // Directly call the frontend to stop the clip and close out the XML file. There is
        // a race condition between DDC and DAQ, so it will never signal to handleClipRecordingStopped
        // and the XML will not be written. We can explicitly call the XML file shutdown and give a
        // dummy value for clipRecordCount. Currently the clipRecordCount isn't being used in consoleApp
        // or HomeScreen.
        on_recordLoopButton_clicked();
        handleClipRecordingStopped();
    }

    // stop any clip that is playing
    if( isClipPlaying )
    {
        closePlayback();
    }

    // get capture and loop statistics for the case before session ends
    sessionDatabase db; //lcv = sessionDatabase::Instance();
    int numCaptures = db.getNumCaptures();
    sessionDatabase::LoopStat loopStat = db.getLoopsStats();

    LOG( INFO, QString( "Number of Captures = %1" ).arg( numCaptures ) )
    LOG( INFO, QString( "Number of Loops = %1" ).arg( loopStat.numLoops ) )
    LOG( INFO, QString( "Total Length of Loops (ms) = %1" ).arg( loopStat.totLength ) )

    // Close out the session.  This sends out a message to the event log.
    session.shutdown();

    // give pause for any messages in flight and then continue shutdown
    Sleep( 1000 );
}

/*
 * mouseTimerExpiry
 *
 * Check the mouse position periodically to make sure
 * it's within the tech window.
 */
void frontend::mouseTimerExpiry(void)
{
    /*
     * It is possible to be on the Physician screen for < HideMouseDelay_ms;
     * in that case, the mouse can get stuck just outside of the
     * Technician monitor. Prevent the mouse from going below the bottom of
     * the Technician screen while momentarily on the Physician screen.
     */
    if( QCursor::pos().y() > ( wmgr->getTechnicianDisplayGeometry().height() - 1 ) )
    {
        QCursor::setPos( ( QCursor::pos().x() ), ( wmgr->getTechnicianDisplayGeometry().height() - 1 ) );
    }

    // Check the cursor position, if it's off the window, bring it back!
    if( QCursor::pos().x() > ( wmgr->getTechnicianDisplayGeometry().width() - 1 ) )
    {
        QCursor::setPos( ( wmgr->getTechnicianDisplayGeometry().width() - 1 ), QCursor::pos().y() );
    }
    update();
}

/*
 * clockTimerExpiry
 *
 * Update the clock on the screen
 *
 */
void frontend::clockTimerExpiry()
{
    ui.timeFieldLabel->setText( QTime::currentTime().toString() );
    ui.timeFieldLabel_mini->setText( QTime::currentTime().toString() );
//lcv    docWindow->ui.timeFieldLabel->setText( QTime::currentTime().toString() );
    auxMon->updateTime( QTime::currentTime().toString() );

    update();
}

/*
 * hideMouseTimerExpiry
 *
 * Hide the mouse cursor after a period of mouse inactivity. Calls
 * to setOverrideCursor() must have a corresponding call to
 * restoreMouseCursor(); this is in eventFilter(). Don't hide the cursor
 * if the active window is a widget other than the frontend window.
 */
void frontend::hideMouseTimerExpiry()
{
    if( this->isActiveWindow() )
    {
        qApp->setOverrideCursor( QCursor( Qt::BlankCursor ) );
        hideMouseTimer.stop();
        QToolTip::hideText(); // Hide any tooltip that might exist at the same time.
    }
}

/*
 * storageSpaceTimerExpiry
 *
 * Check the available storage space and take action if it is low. Stop
 * checking space if this is triggered.
 *
 */
void frontend::storageSpaceTimerExpiry()
{
    // The L in front of the string does some WINAPI magic to convert
    // a string literal into a Windows LPCWSTR beast.
#define ROOT_PATH L"c:"

    if( SawFile::getDiskFreeSpaceInGB( ROOT_PATH ) < MinDriveSpace_GB )
    {
        // stop any ongoing recording
        if( isRecordingClip )
        {
            on_recordLoopButton_clicked();
        }

        disableStorage( true );

        // Stop checking. This will prevent additional warnings popping up
        disconnect( &storageSpaceTimer, SIGNAL(timeout()), this, SLOT(storageSpaceTimerExpiry()) );
    }
}

/*
 * reenableRecordLoopButtonExpiry
 */
void frontend::reenableRecordLoopButtonExpiry( void )
{
    preventFastRecordings = false;
    preventFastRecordingsTimer.stop();
    ui.recordLoopButton->setEnabled( true );
}

/*
 * handleError
 *
 * Display a notice regarding a fatal error from another object.
 * Initiate cleanup after user dismisses.
 */
void frontend::handleError( QString notice )
{
    // Call the system-wide error handler. This function does not return
    displayFailureMessage( notice, true );
}

/*
 * startDataCapture
 *
 * Start up the Data Consumer thread as long as a Data Consumer object has been created
 */
void frontend::startDataCapture( void )
{
    if( consumer )
    {
        consumer->start();
    }
}

/*
 * stopDataCapture
 *
 * Shut down the Data Consumer thread if one exists
 */
void frontend::stopDataCapture( void )
{
    if( consumer )
    {
        consumer->stop();
        consumer->wait();
    }
}


/*
 * startDaq
 *
 * Start up the DAQ thread as long as a DAQ object has been created
 */
void frontend::startDaq( void )
{
    if( idaq )
    {
        qDebug() << "frontend::startDAQ()";
        idaq->start();
    }
}

/*
 * pauseDaq
 *
 * Stop the low-level data processing without shutting down the entire DAQ
 */
void frontend::pauseDaq( void )
{
//    qDebug() << "frontend::pauseDAQ()";
    if( idaq )
    {
        idaq->pause();
    }
}

/*
 * resumeDaq
 *
 * Restart data processing
 */
void frontend::resumeDaq( void )
{
//    qDebug() << "frontend::resumeDAQ()";
    if( idaq )
    {
        idaq->resume();
    }
}

/*
 * stopDaq
 *
 * Shut down the DAQ thread if one exists
 */
void frontend::stopDaq( void )
{
//    qDebug() << "frontend::stopDAQ()";
    if(idaq)
    {
        idaq->stop();
        idaq->wait( DaqWaitTimeout_ms );
    }
}

/*
 * handleWarning
 *
 * Display a dismissable notice regarding an informational event from
 * another object.
 */
void frontend::handleWarning( QString notice )
{
    // Call the system-wide warning handler.
    displayWarningMessage( notice );
}

/*
 * on_advancedViewButton_clicked()
 *
 * Display the advanced view screen upon user request.
 */
void frontend::on_advancedViewButton_clicked()
{
    /*
     * Hide the Display Options window if it is showing
     */
    if( viewOption->isVisible() )
    {
        on_displayOptionsButton_clicked();
        ui.displayOptionsButton->setChecked( false );
    }

    /*
     * Toggle the advanced view
     */
    if( advView->isVisible() )
    {
        advView->hide();
        LOG( INFO, "Advanced View: stop" )
    }
    else
    {
        // Send in the proper sizes for the displays
        advView->handleRawDataLengthChange( idaq->getRecordLength() );
        qDebug() <<  " getRecordLength = " << idaq->getRecordLength() << endl;

        advView->show();
        emit checkLaserDiodeStatus();
        LOG( INFO, "Advanced View: start" )
    }
}

/*
 * on_displayOptionsButton_clicked()
 *
 * Display view options screen upon user request.
 */
void frontend::on_displayOptionsButton_clicked()
{
    /*
     * Hide the Advanced View window if it is showing
     */
    if( advView->isVisible() )
    {
        on_advancedViewButton_clicked();
        ui.advancedViewButton->setChecked( false );
    }

    if( viewOption->isVisible() )
    {
        viewOption->hide();
        LOG( INFO, "Display Options: stop" )
    }
    else
    {
        viewOption->show();
        LOG( INFO, "Display Options: start" )
    }
}

/*
 * on_scanSyncButton_clicked()
 *
 * Start the lag adjustment wizard upon request.
 */
void frontend::on_scanSyncButton_clicked()
{
    bool wasClipRecording = false;

    if( isRecordingClip )
    {
        wasClipRecording = true;
        on_recordLoopButton_clicked(); // Stop the recording
    }
    // Turn off zooming
    on_zoomResetPushButton_clicked();

    if( isAnnotateOn )
    {
        on_annotateImagePushButton_clicked();
        ui.annotateImagePushButton->setChecked( false );
    }

    // make sure the image is live
    m_scene->dismissReviewImages();

    // Turn off mouse rotation of sector
    emit disableMouseRotateSector();
    LOG( INFO, "Scan Sync Adjustment started" )

    userSettings &settings = userSettings::Instance();
    ui.scanSyncButton->setChecked( true );

    handleStatusText( tr( "Scan Sync setup" ) );

    emit tagEvent( "Scan Sync Adjustment Start" );

    if( lagHandler )
    {
        return;
    }

    // Remove current lag correction value
    double prevWindAngle = m_scene->getWindAngle();
    m_scene->setWindAngle( 0 );

    // Start from a fresh uncluttered screen, focus on the sector
    m_scene->clearImages();

    // create a new window (no parent) so the UI is modal
    lagHandler = new lagWizard;
    connect( consumer,   SIGNAL(directionOfRotation(directionTracker::Direction_T)),
             lagHandler, SLOT(handleDirectionChange()) );
    connect( m_scene,      SIGNAL(fullRotation()),
             lagHandler, SLOT(handleFullRotation()) );
    connect( lagHandler, SIGNAL(resetIntegrationAngle()),
             m_scene,      SLOT(resetIntegrationAngle()) );
    lagHandler->setScene( m_scene );

    m_scene->handleLagWizardStart();

    // determine how to center the wizard on the primary screen
    int x = ( wmgr->getTechnicianDisplayGeometry().width()  - lagHandler->width()  ) / 2;
    int y = ( wmgr->getTechnicianDisplayGeometry().height() - lagHandler->height() ) / 2;

    // Force the wizard to the primary monitor
    lagHandler->setGeometry( x, y, lagHandler->width(), lagHandler->height() );

    int result = lagHandler->exec();

    if( result == QDialog::Accepted )
    {
        const double NewLagAngle = lagHandler->getAngle();

        m_scene->setWindAngle( NewLagAngle );
        settings.setLag( int(NewLagAngle) );
        emit sendLagAngle( NewLagAngle );
        emit tagEvent( "Scan Sync = " + QString( "%1" ).arg( NewLagAngle ) );
        LOG( INFO, QString( "Scan Sync = %1" ).arg( NewLagAngle ))
    }
    else
    {
        // cancelled: reset the previous lag correction
        m_scene->setWindAngle( prevWindAngle );

        emit tagEvent( "Scan Sync Adjustment Cancelled" );
        LOG( INFO, "Scan Sync Adjustment Cancelled" )
    }

    m_scene->resetRotationCounter();
    m_scene->handleLagWizardStop();

    delete lagHandler;
    lagHandler = nullptr;

    ui.scanSyncButton->setChecked( false );
    handleStatusText( tr( "LIVE" ) );

    // Re-enable mouse rotation of sector
    emit enableMouseRotateSector();

    // Start a new recording if it was recording before ScanSync started
    if( wasClipRecording )
    {
        on_recordLoopButton_clicked();
    }
}

/*
 * handleManualLagAngle
 *
 * Update the m_scene settings if the manual scan sync was used
 */
void frontend::handleManualLagAngle( double newAngle )
{
    m_scene->setWindAngle( newAngle );
    emit tagEvent( "Scan Sync = " + QString( "%1" ).arg( newAngle ) );
    LOG( INFO, QString( "Scan Sync = %1" ).arg( newAngle ))
}

/*
 * on_deviceSelectButton_clicked()
 *
 * Start the device selection wizard to setup the catheter imaging parameters.
 */
QDialog::DialogCode frontend::on_deviceSelectButton_clicked()
{
    LOG( INFO, "Device Selection: Start" )
    emit tagEvent( "Device Change Started" );
    handleStatusText( tr( "Changing Devices" ) );
    ui.deviceSelectButton->setChecked( true );

    SledSupport &sled = SledSupport::Instance();
//	sled.stopSled();		// stop sled if running
//    sled.stop();			// stop sled processing

    if( isAnnotateOn )
    {
        on_annotateImagePushButton_clicked();
        ui.annotateImagePushButton->setChecked( false );
    }

    deviceWizard dWiz;

    // determine how to center the wizard on the primary screen
    int x = ( wmgr->getTechnicianDisplayGeometry().width() - dWiz.width() )   / 2;
    int y = ( wmgr->getTechnicianDisplayGeometry().height() - dWiz.height() ) / 2;

    // Force the wizard to the primary monitor
    dWiz.setGeometry( x, y, dWiz.width(), dWiz.height() );

    // Temporarily pause data while we start
    stopDataCapture();

    deviceSettings &dev = deviceSettings::Instance();
    int previousDevice = dev.getCurrentDevice();

    // Turn off updates for the main screens while this is running
    QGraphicsView::ViewportUpdateMode oldTechMode = ui.liveGraphicsView->viewportUpdateMode();
    ui.liveGraphicsView->setViewportUpdateMode( QGraphicsView::NoViewportUpdate );

    QGraphicsView::ViewportUpdateMode oldDocMode = docWindow->ui.liveGraphicsView->viewportUpdateMode();
    docWindow->ui.liveGraphicsView->setViewportUpdateMode( QGraphicsView::NoViewportUpdate );
    ui.zoomSlider->setValue( ui.zoomSlider->minimum() );

    /*
     * If this is the first time the device wizard has been called, make sure a device
     * is configured. If the wizard is cancelled, set to the previously selected device,
     * or return as QDialog::Rejected if no previous device exists.
     */
    QDialog::DialogCode retVal = QDialog::DialogCode(dWiz.exec());
    if( retVal == QDialog::Rejected )
    {
        // User cancelled the device change, revert to the previously selected device.
        LOG( INFO, "Device change cancelled." )

        // If no device has been selected, then this is the first pass through and the user wants to return to Homescreen.
        if( previousDevice == -1 )
        {
            return retVal;
        }
    }
    else
    {
        emit recordBackgroundData( false );
        shutdownHardware();
        LOG( INFO, QString( "Current Device: %1" ).arg( dev.getCurrentDeviceName() ) )

        // Restore updates
        m_scene->clearImages();
        ui.liveGraphicsView->setViewportUpdateMode( oldTechMode );
        docWindow->ui.liveGraphicsView->setViewportUpdateMode( oldDocMode );
    }
    return retVal;
}

void frontend::startDAQprepareView()
{
    startDaq();
    viewOption->setLagAngleToZero();

    // turn on background recording (if enabled) and event log processing
    emit recordBackgroundData( true );
    emit forwardTurnDiodeOn();

    startDataCapture(); // Paused at the start of the wizard, so restart for rejected or accepted wizard state.

    deviceSettings &dev = deviceSettings::Instance();
    if( dev.current() )
    {
        emit tagEvent( QString( "Device Changed to: %1,%2,%3" ).arg( dev.getCurrentDeviceName() ).arg( dev.current()->getInternalImagingMask_px() ).arg( dev.current()->getCatheterRadius_px() ) );
    }

    // Want an indicator with 5 positions for High Speed, will be disabled if a LS device is chosen.
    depthSetting &ds = depthSetting::Instance();
    ui.imagingDepthWidget->init( VariableDepthNumChunks, ds.getImagingDepth_S(), "DEPTH", ds.getMinDepth_px(), ds.getMaxDepth_px() );
    ui.imagingDepthWidget->setToolTip( "Change Imaging Depth." );
    configureControlsForCurrentDevice();

    ui.deviceSelectButton->setChecked( false );
    handleStatusText( tr( "LIVE" ) );
    emit tagEvent( "Device Change End" );
    LOG( INFO, "Device Selection: End" )

    setupDeviceForSledSupport();
}

/*
 * handleShowCurrentDeviceLabel
 *
 * Update the device label when returning to the live view from capture review.
 */
void frontend::handleShowCurrentDeviceLabel()
{
    deviceSettings &dev = deviceSettings::Instance();
    docWindow->setDeviceName( dev.current()->getDeviceName() );
}

/*
 * handleDeviceChange()
 *
 * We've gotten a signal that the device has changed, update the tech screen to
 * reflect the new name.
 */
void frontend::handleDeviceChange()
{
//	qDebug() << "**** frontend::handleDeviceChange()";
    deviceSettings &dev = deviceSettings::Instance();
    ui.deviceFieldLabel->setText( dev.current()->getDeviceName() );
    docWindow->setDeviceName( dev.current()->getDeviceName() );
    auxMon->setDeviceName( dev.current()->getDeviceName() );
/*
    SledSupport &sledSupport = SledSupport::Instance();
    if( devSettings.current()->isBidirectional())
    {
        sledSupport.setSledRotation(-1);	// No direction indicator
    }
*/
    // Make sure the next device is started with Zoom off.
    on_zoomResetPushButton_clicked();
}

/*
 * updateCaseInfo
 *
 * Update any UI elements that refer to case information
 */
void frontend::updateCaseInfo()
{
    caseInfo &info = caseInfo::Instance();

    // Update UI
    ui.patientIDFieldLabel->setText( info.getPatientID() );

    LOG( INFO, QString( "Patient ID added or updated" ) )
    LOG( INFO, QString( "Case ID: %1" ).arg( info.getCaseID() ) )
}

/*
 * on_caseDetailsButton_clicked()
 *
 * Popup (or dismiss) the case details editor.
 */
void frontend::on_caseDetailsButton_clicked()
{
    LOG( INFO, "Case details button clicked" )

//    // Update session case information
//    if( setupCase( false ) )
//    {
//        // Update information on the screen
//        updateCaseInfo();

//        LOG( INFO, "Case details modified" )
//    }
//    else
//    {
//        LOG( INFO, "Case details were not changed")
//    }
}

/*
 * on_recordLoopButton_clicked
 *
 *
 */
void frontend::on_recordLoopButton_clicked()
{
    if( preventFastRecordings )
    {
        /*
         * Do not allow toggling of the record state faster than MinRecordingLength_ms
         */
        LOG( DEBUG, "Prevented fast recording switch" )
    }
    else if( !isRecordingClip )
    {
#if ENABLE_SINGLE_STREAM_RECORDING
        // Turn off background recording to reduce CPU load
        emit recordBackgroundData( false );
#endif

        // start recording a new clip
        caseInfo       &info        = caseInfo::Instance();
        deviceSettings &dev = deviceSettings::Instance();

        ui.recordLoopButton->setDisabled( true );
        ui.deviceSelectButton->setDisabled( true );
        viewOption->disableButtons();
        ui.reviewWidget->disableClipSelection();
        ui.recordingLabel->show();
        docWindow->ui.recordingLabel->show();
        auxMon->setText( AuxMonitor::Recording, true );
//        ui.loopMaskLabel->show();

        // record the start time
        clipTimestamp = QDateTime::currentDateTime().toUTC();

        // set up file storage and start recording
        currClipNumber++;
        strClipNumber = QString( "%1" ).arg( currClipNumber, 3, 10, QLatin1Char( '0' ) );

        isRecordingClip = true;
        clipTimer.start();

        // Construct the target base name
        const QString ClipName = "clip-" + strClipNumber;
        QString clipFilename = info.getClipsDir() + "/" + ClipName;

        emit setClipFilename( clipFilename );
        emit startClipRecording();
        emit captureClipImages( strClipNumber );

        userSettings &user = userSettings::Instance();
        QString viewStr;
        if( user.isDistalToProximalView() )
        {
            viewStr = "DistalToProximal";
        }
        else
        {
            viewStr = "ProximalToDistal";
        }
        LOG( INFO, "Clip Recording: Start (clip-" + strClipNumber + ")" )
        /*
         * Save clip information to the session database immediately. This
         * ensures that Case Review will see the clip even if the console
         * application crashes.
         */
        clipListModel &clipList = clipListModel::Instance();
        clipList.addClipCapture( ClipName,
                                 clipTimestamp.toTime_t(),
                                 viewStr,
                                 dev.current()->getDeviceName());

        preventFastRecordingsTimer.start( MinRecordingLength_ms );
        preventFastRecordings = true;
    }
    else
    {
        // disable access to the button until the loop is completely saved
        ui.recordLoopButton->setDisabled( true );

        // Stop recording
        emit stopClipRecording();
    }
}

/*
 * handleClipRecordingStopped
 *
 * Re-enable the UI and close out the clip
 */
void frontend::handleClipRecordingStopped( void )
{
    LOG( INFO, "Clip Recording: Stop (clip-" + strClipNumber + ")" )

    // stop recording the clips
    ui.deviceSelectButton->setDisabled( false );
    ui.recordingLabel->hide();
    docWindow->ui.recordingLabel->hide();
    auxMon->setText( AuxMonitor::Recording, false );
//    ui.loopMaskLabel->hide();

    viewOption->enableButtons();
    ui.reviewWidget->enableClipSelection();

    isRecordingClip = false;

    // Update timing information
    clipListModel &clipList = clipListModel::Instance();

    // clipLength_ms is updated when the video recording is closed
    clipList.updateClipInfo( clipLength_ms );

    LOG( INFO, QString( "Clip Recording: Length: %1 ms (clip-%2)" ).arg( clipLength_ms ).arg( strClipNumber ) )
//    qDebug() << "Clip" << strClipNumber << "length (ms): " << clipLength_ms;


#if ENABLE_SINGLE_STREAM_RECORDING
    if( !isShuttingDown )
    {
        // Restart background recording if it is enabled and the system is not shutting down
        emit recordBackgroundData( true );
    }
#endif

    // only re-enable the button if there is space on the drive
    if( !disableCapturing )
    {
        ui.recordLoopButton->setEnabled( true );
    }
}

/*
 * handleEndOfFile
 *
 * reset the clip for re-playing when it reaches the end of its timeline
 */
void frontend::handleEndOfFile()
{
    if( !ui.transportWidget->isRepeatChecked() )
    {
        handlePauseButton_clicked();
        emit forcePauseButtonOff();
    }
    else
    {
        m_scene->restartLoop();
    }
}

/*
 * updateCatheterViewLabel
 */
void frontend::updateCatheterViewLabel()
{
    QString str( "" );
    userSettings &user = userSettings::Instance();
    if( user.isDistalToProximalView() )
    {
        str = CatheterPointedDownText;
    }
    else
    {
        str = CatheterPointedUpText;
    }
    ui.catheterViewLabel->setText( str );
//lcv    docWindow->ui.catheterViewLabel->setText( str );
    auxMon->setText( AuxMonitor::CatheterView, true, str );
}

/*
 * handleLoopLoaded
 */
void frontend::handleLoopLoaded( QString loopFilename )
{
    /*
     * Only
     * check this the first time a loop is loaded so the state is correctly restored.
     */
    if( !isLoopLoaded )
    {
        isLoopLoaded = true;
    }

    if( isClipPlaying )
    {
        handlePauseButton_clicked();
    }

    // Stop the hardware and switch over to the stored clip
    pauseDaq();

    // set up for clip playback
    playbackClipName = loopFilename;
    emit setClipName( loopFilename );
    m_scene->setClipForPlayback( loopFilename );

    emit forcePauseButtonOff();

    configureDisplayForReview();

    playbackControlsVisible( true );
}
/*
 * handlePlayButton_clicked
 *
 * Configure the hardware and UI for replaying an OCT Loop.  The hardware is
 * put into a state that stops sending data to the frontend; the UI is switches
 * to displaying data from the storage device.  Full case recording continues in
 * the background but it is not receiving new data from the hardware so the movies
 * do not update.
 */
void frontend::handlePlayButton_clicked()
{
    if( isLoopLoaded && !isClipPlaying )
    {
        if( isImageCaptureLoaded )
        {
            m_scene->dismissReviewImages();
        }

        m_scene->resetSector();

        m_scene->startPlayback();
        isClipPlaying = true;

        LOG( INFO, QString( "Clip playback: started (%1)" ).arg( playbackClipName ) )
    }
}

/*
 * handlePauseButton_clicked
 */
void frontend::handlePauseButton_clicked()
{
    isClipPlaying = false;
    m_scene->pausePlayback();
    LOG( INFO, "Clip playback: paused" )
}

/*
 * closePlayback
 *
 * Switch the UI and hardware back to normal use.
 */
void frontend::closePlayback()
{
    LOG( INFO, "Loop playback stopped" )
    deviceSettings &dev = deviceSettings::Instance();

    isClipPlaying = false;
    handleStatusText( tr( "LIVE" ) );
    playbackControlsVisible( false );

    // reset the view status to the current settings
    docWindow->setDeviceName( dev.current()->getDeviceName() );
    ui.physicianPreviewButton->setEnabled( true ); // re-enable the button

    enableCaptureButtons();

    m_scene->resetSector();
    m_scene->clearImages();
    m_scene->stopPlayback();

    resumeDaq();

    LOG( INFO, "Clip playback: closed" )
}

/*
 * configureControlsForCurrentDevice
 *
 * Set the enabled/disabled state of buttons based on the type
 * of catheter being used.
 */
void frontend::configureControlsForCurrentDevice()
{
    deviceSettings &dev = deviceSettings::Instance();
    ui.scanSyncButton->setDisabled( true );
    ui.imagingDepthWidget->enableControls( true );
}

/*
 * playbackControlsVisible
 */
void frontend::playbackControlsVisible( bool state )
{
    // show or hide the playback controls
    ui.transportWidget->setVisible( state );
    ui.playbackGroupBox->setVisible( state );
}

/*
 * handleStatusText
 */
void frontend::handleStatusText( QString status )
{
    ui.label_live->setText( status );
//lcv    docWindow->ui.statusLabel->setText( status );
    auxMon->setText( AuxMonitor::Status, true, status );
}

/*
 * configureClock
 *
 * Set up the onscreen clocks: colors and event connection
 *
 */
void frontend::configureClock( void )
{
    connect( &clockTimer, SIGNAL(timeout()), this, SLOT(clockTimerExpiry()) );
    docWindow->ui.timeFieldLabel->setStyleSheet( "QFrame { color: white }" );
}

/*
 * eventFilter
 *
 * Filter events as needed
 */
bool frontend::eventFilter( QObject *obj, QEvent *event )
{
    /*
     * Turn the mouse capture timer on if the application is active; otherwise,
     * turn it off. The application is deactivated if the screen saver starts.
     */
    if( obj == qApp )
    {
        if( event->type() == QEvent::ApplicationActivate )
        {
            captureMouse( true );
        }

        if( event->type() == QEvent::ApplicationDeactivate )
        {
            captureMouse( false );
        }
    }

    /*
     * Monitor mouse movement. If the mouse has been hidden, show it and start
     * the timer for the next check for hiding it.
     */
    if( event->type() == QEvent::MouseMove )
    {
        qApp->restoreOverrideCursor();
        hideMouseTimer.start( HideMouseDelay_ms );
    }

    return QWidget::eventFilter( obj, event );
}

/*
 * keyPressEvent
 *
 * Handles keypresses in the main window. Auto-repeat key presses are ignored
 * for all keys except the Space Key. The Space key cannot be captured this way
 * since the window manager uses it to activate the currently selected button;
 * defining it as a shortcut in the UI file bypasses that.
 */
void frontend::keyPressEvent( QKeyEvent *event )
{
    if( event->isAutoRepeat() )
    {
        event->ignore();
    }
    else if( event->key() == Qt::Key_0 )
    {
        if( !disableCapturing )
        {
            on_captureImageButton_clicked();
        }
        event->accept();
    }
}

/*
 * captureMouse
 *
 * Enable or disable the mouse capture timer.
 *
 */
void frontend::captureMouse( bool /*isEnabled*/ )
{
#if USE_MOUSE_CAPTURE
    if( isEnabled )
    {
        connect( &mouseCaptureTimer, SIGNAL(timeout()), this, SLOT(mouseTimerExpiry()) );
    }
    else
    {
        disconnect( &mouseCaptureTimer, SIGNAL(timeout()), this, SLOT(mouseTimerExpiry()) );
    }
//#else
//    isEnabled; // quiet the compiler warnings
#endif
}

/*
 * enableCaptureButtons
 */
void frontend::enableCaptureButtons()
{
    ui.captureImageButton->setEnabled( true );
    ui.recordLoopButton->setEnabled( true );
}

/*
 * disableCaptureButtons
 */
void frontend::disableCaptureButtons()
{
    ui.captureImageButton->setDisabled( true );
    ui.recordLoopButton->setDisabled( true );
}


/*
 * hideCatheterView
 *
 */
void frontend::hideCatheterView( void )
{
    ui.line->hide();
    ui.catheterViewLabel->hide();
    docWindow->ui.catheterViewLabel->hide();
    auxMon->setText( AuxMonitor::CatheterView, false );
}

/*
 * showCatheterView
 *
 */
void frontend::showCatheterView( void )
{
    ui.line->show();
    ui.catheterViewLabel->show();
    docWindow->ui.catheterViewLabel->show();
    auxMon->setText( AuxMonitor::CatheterView, true );
}

/*
 * setSceneCursor
 *
 * Simplify switching the state of the mouse shown over Live Scene
 */
void frontend::setSceneCursor( QCursor cursor )
{
    ui.liveGraphicsView->viewport()->setProperty( "cursor", QVariant( cursor ) );
}

/*
 * handleReviewDeviceName
 */
void frontend::handleReviewDeviceName( QString str )
{
    ui.deviceFieldLabel->setText( str );
}

void frontend::setIDAQ(IDAQ *object)
{
    idaq = object;

    IDAQ* signalSource(nullptr);

    if(object->getSignalSource()){
        signalSource = object->getSignalSource();
    } else {
        signalSource = object;
    }

    ui.liveGraphicsView->setScene( m_scene );
    ui.liveGraphicsView->fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
    centerLiveGraphicsView(); // center the panning position of the view over the sector

    if(signalSource)
    {
        connect( viewOption, SIGNAL(currFrameWeight_percentChanged(int)), SignalModel::instance(), SLOT(setCurrFrameWeight_percent(int)) );

        connect( m_scene, SIGNAL(sendDisplayAngle(float)), signalSource, SIGNAL(handleDisplayAngle(float)) );

        // connect error/warning handlers before initializing the hardware
        connect( signalSource, SIGNAL( sendWarning( QString ) ),       this,    SLOT( handleWarning( QString ) ) );
        connect( signalSource, SIGNAL( sendError( QString ) ),         this,    SLOT( handleError( QString ) ) );
        connect( signalSource, SIGNAL( signalDaqResetToFrontend() ),   this,    SLOT( handleDaqReset() ) );

        // daq to advView
        connect( signalSource, SIGNAL( frameRate(int) ),               advView, SLOT( updateDaqUpdatesPerSecond(int) ) );
        connect( signalSource, SIGNAL( attenuateLaser(bool) ),         advView, SLOT( attenuateLaser(bool) ) );

        // frontend controls to daq
        connect( this,       SIGNAL( brightnessChange( int ) ),    SignalModel::instance(),  SLOT( setBlackLevel( int ) ) );
        connect( this,       SIGNAL( contrastChange( int ) ),      SignalModel::instance(),  SLOT( setWhiteLevel( int ) ) );
        connect( this,       SIGNAL( brightnessChange( int ) ),    signalSource,  SIGNAL( setBlackLevel( int ) ) );
        connect( this,       SIGNAL( contrastChange( int ) ),      signalSource,  SIGNAL( setWhiteLevel( int ) ) );

        // view option controls to daq
        connect( viewOption, SIGNAL( enableAveraging( bool ) ),    SignalModel::instance(),   SLOT( setIsAveragingNoiseReduction( bool ) ) );
        connect( viewOption, SIGNAL( enableInvertColors( bool ) ), SignalModel::instance(),   SLOT( setIsInvertColors( bool ) ) );

        // view options to set color mode
        connect( viewOption, SIGNAL( setColorModeGray() ),         m_scene, SLOT( loadColorModeGray() ) );
        connect( viewOption, SIGNAL( setColorModeSepia() ),        m_scene, SLOT( loadColorModeSepia() ) );

        connect( advView, SIGNAL( tdcToggled(bool) ), signalSource, SLOT(enableAuxTriggerAsTriggerEnable(bool) ) ); // * R&D only

        connect( advView, SIGNAL( tdcToggled(bool) ), signalSource, SLOT(enableAuxTriggerAsTriggerEnable(bool) ) ); // * R&D only
    }

    {
        depthSetting &depthManager = depthSetting::Instance();
        // connect the level gauge UI element with the depthSettings singleton object
        connect( ui.imagingDepthWidget, SIGNAL( valueChanged(double) ), &depthManager, SLOT( updateImagingDepth(double) ) );
//        connect( m_formL300, SIGNAL( depthChanged(double) ), &depthManager, SLOT( updateImagingDepth(double) ) );
        ui.imagingDepthWidget->init( 5, 3, "DEPTH", 1, 5 ); // dummy settings that will be overwritten at device selection
        ui.imagingDepthWidget->setEnabled( true );

    }

    if(idaq){
        if(idaq->getSignalSource()){
            connect( idaq->getSignalSource(), &IDAQ::updateSector, this, &frontend::updateSector);
            connect( idaq->getSignalSource(), &IDAQ::notifyAcqData, advView, &advancedView::handleAcqData);
        }
        idaq->init();
    }
    // Sync the view options from the System.ini
    viewOption->updateValues();

    startDAQprepareView();
}

void frontend::showSpeed(bool isShown)
{
    m_mainScreen->showSpeed(isShown);
}

/*
 * shutdownHardware
 */
void frontend::shutdownHardware( void )
{
    if( idaq )
    {
        stopDataCapture();
        stopDaq();

        delete idaq;
        idaq = nullptr;
    }

    // Turn off the laser diode when the hardware is not running
    emit forwardTurnDiodeOff();

    /*
     * About 500 ms sleep is required to fully shut down the hardware, or
     * else starting data collection immediately after will result in the first
     * several records being high intensity, saturated data. This is not fully
     * understood, but it is likely related to arming/disarming the Alazar DAQ.
     */
    Sleep( 500 );
}

#if ENABLE_COLORMAP_OPTIONS
// R&D only
void frontend::on_contrastCurveButton_clicked()
{
    if( !curveDlg )
    {
        curveDlg = new curvesDialog(this);
    }

    connect( curveDlg, SIGNAL(accepted()), this, SLOT(curvesDialogFinished()) );
    connect( curveDlg, SIGNAL(rejected()), this, SLOT(curvesDialogFinished()) );
    connect( curveDlg, SIGNAL(updateCurveMap()), this, SLOT(curveMapChanged()) );

    curveDlg->show();
}

// R&D only
void frontend::curvesDialogFinished()
{
    m_scene->updateGrayScaleMap( curveDlg->getMap() );
    curveDlg->hide();
}

// R&D only
void frontend::curveMapChanged(void)
{
    m_scene->updateGrayScaleMap( curveDlg->getMap() );
}

/*
 * populateColormapList
 *
 * Add all available colormaps
 * R&D only
 */
void frontend::populateColormapList( void )
{
    QDir dir( SystemDir + "/colormaps" );
    QStringList filters;
    filters << "*.csv";
    QFileInfoList list = dir.entryInfoList( filters );

    for( int i = 0; i < list.size(); i++ )
    {
        ui.colormapListWidget->addItem( list.at( i ).baseName() );
    }
}

// R&D
void frontend::on_colormapListWidget_doubleClicked( const QModelIndex &index )
{
    m_scene->loadColormap( SystemDir + "/colormaps/" + index.data().toString() + ".csv" );
    ui.colormapLabel->setPixmap( QPixmap::fromImage( sampleMap ) );
}
#endif

#if ENABLE_VIDEO_CRF_QUALITY_TESTING
// R&D -- global variable
QString gCrfTestValue( "25" );

// R&D
void frontend::on_crfTestSpinBox_valueChanged( const QString &arg1 )
{
    gCrfTestValue = arg1;
}
#endif

/*
 * on_measureModePushButton_clicked
 */
void frontend::on_measureModePushButton_clicked()
{
    if( measureModeAllowed )
    {
        // Toggle measurement mode
        isMeasureModeActive = !isMeasureModeActive;
        setMeasurementMode( isMeasureModeActive );
    }
    else
    {
        displayWarningMessage( QString( "Measurements are disabled for the current device profile.\n\nPlease contact Avinger Service at %1." ).arg( ServiceNumber ) );
        ui.measureModePushButton->setChecked( false );
    }
}

/*
 * setMeasurementMode
 *
 * Show or hide the measurement groupbox, overlay, and label.
 */
void frontend::setMeasurementMode( bool enable )
{
    if( enable )
    {
        m_scene->setMeasureModeArea( true, Qt::magenta );
        setSceneCursor( QCursor( Qt::CrossCursor ) );
        ui.liveGraphicsView->setToolTip( "" );
        ui.measureModePushButton->setChecked( true );
        LOG( INFO, "Measure Mode: start" )
    }
    else
    {
        m_scene->setMeasureModeArea( false, Qt::magenta );
        setSceneCursor( QCursor( Qt::OpenHandCursor ) );
        ui.measureModePushButton->setChecked( false );
        LOG( INFO, "Measure Mode: stop" )
    }
    isMeasureModeActive = enable; // state variable for toggle action
}

/*
 * enableDisableMeasurementForCapture
 *
 * Test the value to see if it is valid. Any value <= 0 is invalid and
 * will not allow the measure mode. The value will be set to -1 in
 * livescene at time of capture if the device measurement version does
 * not support the supported system measurement version.
 */
void frontend::enableDisableMeasurementForCapture( int pixelsPerMm )
{
    if( pixelsPerMm <= 0 )
    {
        measureModeAllowed = false;
        setMeasurementMode( false );
    }
    else
    {
        measureModeAllowed = true;
    }
}

void frontend::updateSector(OCTFile::OctData_t* frameData)
{

    if(frameData && m_scene && m_scanWorker){

        const auto* sm =  SignalModel::instance();

        QImage* image = m_scene->sectorImage();

        frameData->dispData = image->bits();
        auto bufferLength = sm->getBufferLength();

        m_scanWorker->warpData( frameData, bufferLength);

        if(m_scanWorker->isReady){

            if(image && frameData && frameData->dispData){
                QGraphicsPixmapItem* pixmap = m_scene->sectorHandle();

                if(pixmap){
                    QPixmap tmpPixmap = QPixmap::fromImage( *image, Qt::MonoOnly);
                    pixmap->setPixmap(tmpPixmap);
                }
//                m_scene->setDoPaint();
            }
        }
    }
}

/*
 * on_saveMeasurementButton_clicked
 *
 * Capture the tech screen when zooming because the overlays in the doc screen might be
 * out of view. This is due to the different sizes and aspect ratios Tech vs Doc. Once the
 * tech screen is captured, it must be scaled to the docscreen size and rotated to be
 * displayed on the docscreen. Also, the scaling from techscreen size to docscreen size
 * requires scaling the zoom factor. This is best done by comparing aspect ratios of the
 * liveGraphicsView of each screen.
 */
void frontend::on_saveMeasurementButton_clicked()
{
    return; //lcv

    // capture the screen and overwrite the decorated image of the current capture.
//    QImage p = QPixmap::grabWidget( docWindow->ui.liveGraphicsView ).toImage();
    QImage p = docWindow->ui.liveGraphicsView->grab().toImage();

    // Take the tech screen view if zoomed because overlays may be out of view on doc screen.
    if( ( ui.zoomSlider->value() / 100.0 ) > 1.0 )
    {
        float scaledImageFactor = 1.0;
        //p = QPixmap::grabWidget( ui.liveGraphicsView ).toImage();
        QImage p = ui.liveGraphicsView->grab().toImage();

        p = p.scaledToWidth( docWindow->ui.liveGraphicsView->height(), Qt::SmoothTransformation );

        QTransform trans;
        trans.rotate( 90 );
        p = p.transformed( trans, Qt::SmoothTransformation );

        float docScreenRatio = float(docWindow->ui.liveGraphicsView->height()) / float(docWindow->ui.liveGraphicsView->width() );
        float techScreenRatio = float(ui.liveGraphicsView->width()) / float(ui.liveGraphicsView->height());
        scaledImageFactor = techScreenRatio / docScreenRatio;

        // update zoom factor if necessary
        ui.reviewWidget->updateZoomFactor( ui.zoomSlider->value() / 100.0f / scaledImageFactor );
    }

    // send image to captureListModel
    ui.reviewWidget->replaceDecoratedImage( p );
}

/*
 * on_zoomSlider_valueChanged
 *
 * Smooth zooming. The slider has values from 100 (1x) to ( max magnification * 100 ).
 * This zooms x and y the same amount and always zooms on the current view center.
 */
void frontend::on_zoomSlider_valueChanged( int value )
{
    double sx = double(value) / 100.0;
    double sy = sx;

    /*
     * Scale and keep the original view size on the Physician screen.
     * Since the Physician screen is rotated 90 deg, the shear factors
     * from the tranform matrix contain the zoom parameters.  Multipling
     * this by the current zoom level keeps the view correct when it
     * returns back to 1:1.
     */
    docWindow->ui.liveGraphicsView->setTransform( QTransform::fromScale( sx * docViewMatrix.m12(), -sy * docViewMatrix.m21() ) );
    docWindow->ui.liveGraphicsView->rotate( 90 );

    /*
     * Scale and keep the original view size on the Technician screen
     */
    ui.liveGraphicsView->setTransform( QTransform::fromScale( sx * techViewMatrix.m11(), sy * techViewMatrix.m22() ) );
    auxMon->setTransformForZoom( QTransform::fromScale( sx * auxViewMatrix.m11(), sy * auxViewMatrix.m22() ),
                                 ( value > 100.0 ) ); // zoomed is true if value is greater than 100.0

    m_scene->setZoomFactor( float(sx) ); // pass the zoom factor to liveScene

    if( value == ui.zoomSlider->minimum() )
    {
        centerLiveGraphicsView();

        /*
         * Back to 1:1 view. Reset view and interactions to normal.
         */
        isZoomModeOn = false;

        ui.liveGraphicsView->setDragMode( QGraphicsView::NoDrag );

        QString strZoom{QString::number( ui.zoomSlider->minimum())};
        ui.zoomFactorLabel->setText( "[1.00x]" );
        ui.zoomResetPushButton->hide();

        docWindow->ui.zoomFactorLabel->hide();
//lcv        docWindow->ui.zoomFactorLabel->setText( "" );

        auxMon->setText( AuxMonitor::ZoomFactor, false, "" );

        if( !( isAnnotateOn || isMeasureModeActive ) )
        {
            ui.liveGraphicsView->setToolTip( defaultSceneToolTip );
            setSceneCursor( QCursor( Qt::OpenHandCursor ) );
        }
        else
        {
            setSceneCursor( QCursor( Qt::CrossCursor ) );
        }

        // don't reset annotation when in the review state
        if( !( isImageCaptureLoaded || isLoopLoaded ) )
        {
            ui.annotateImagePushButton->setEnabled( true );
        }
        else // still in review state
        {
            ui.liveGraphicsView->setToolTip( "" );
        }

        if( isImageCaptureLoaded )
        {
            ui.measureModePushButton->setEnabled( true );
        }
    }
    else
    {
        /*
         * zooming
         */
        isZoomModeOn = true;

        ui.liveGraphicsView->setDragMode( QGraphicsView::ScrollHandDrag );

        // show two digits for the zoom factor
        zoomFactorText = zoomFactorText.setNum( sx, 'f', 2 ).append( "x" );

        ui.zoomFactorLabel->setText( "["+ zoomFactorText + "]" );
//lcv        docWindow->ui.zoomFactorLabel->setText( tr( "Zoom - " ) + zoomFactorText );
        auxMon->setText( AuxMonitor::ZoomFactor, true, tr( "Zoom - " ) + zoomFactorText );
        ui.zoomResetPushButton->show();

        docWindow->ui.zoomFactorLabel->show();

        ui.liveGraphicsView->setToolTip( "" );
        ui.annotateImagePushButton->setDisabled( true );
        ui.measureModePushButton->setDisabled( true );
    }
}

/*
 * centerLiveGraphicsView
 *
 * Reset the panning position to the center of the sector for the next zoom.
 */
void frontend::centerLiveGraphicsView( void )
{
    ui.liveGraphicsView->centerOn( SectorWidth_px / 2, SectorWidth_px / 2 );
    auxMon->liveGraphicsViewCenterOn( SectorWidth_px / 2, SectorWidth_px / 2 );
}

/*
 * handleTechViewHorizontalPan
 *
 * The two views have different scaling of the same m_scene. Since control
 * of the pan is on the Technician screen, the values for panning must be
 * scaled to the Physician screen.  Because of the 90 deg rotation of the
 * Physician screen, horizontal and vertical scrolling are cross-connected.
 */
void frontend::handleTechViewHorizontalPan( int value )
{
    if( ( ui.liveGraphicsView ) && ( docWindow ) )
    {
        int techRange = ui.liveGraphicsView->horizontalScrollBar()->maximum() -
                        ui.liveGraphicsView->horizontalScrollBar()->minimum();

        int docMin    = docWindow->ui.liveGraphicsView->verticalScrollBar()->minimum();
        int docRange  = docWindow->ui.liveGraphicsView->verticalScrollBar()->maximum() - docMin;

        int auxMin = auxMon->getLiveGraphicsViewHorizontalScrollBar()->minimum();
        int auxRange = auxMon->getLiveGraphicsViewHorizontalScrollBar()->maximum() - auxMin;

        if( techRange != 0 )
        {
            int newValue = ( ( value * docRange ) / techRange ) + docMin;
            docWindow->ui.liveGraphicsView->verticalScrollBar()->setValue( newValue );

            // prevent auxMon panning during zoom
            if( !isZooming )
            {
                newValue = ( ( value * auxRange ) / techRange ) + auxMin;
                auxMon->setLiveGraphicsViewHorizontalScrollBar( newValue );
            }
        }
    }
}

/*
 * handleTechViewVerticalPan
 *
 * The two views have different scaling of the same m_scene. Since control
 * of the pan is on the Technician screen, the values for panning must be
 * scaled to the Physician screen.  Because of the 90 deg rotation of the
 * Physician screen, horizontal and vertical scrolling are cross-connected.
 */
void frontend::handleTechViewVerticalPan( int value )
{
    if( ( ui.liveGraphicsView ) && ( docWindow ) )
    {
        int techMax   = ui.liveGraphicsView->verticalScrollBar()->maximum();
        int techRange = techMax - ui.liveGraphicsView->verticalScrollBar()->minimum();

        int docMin    = docWindow->ui.liveGraphicsView->horizontalScrollBar()->minimum();
        int docRange  = docWindow->ui.liveGraphicsView->horizontalScrollBar()->maximum() - docMin;

        int auxMin = auxMon->getLiveGraphicsViewVerticalScrollBar()->minimum();
        int auxRange = auxMon->getLiveGraphicsViewVerticalScrollBar()->maximum() - auxMin;

        if( techRange != 0 )
        {
            // ( techMax - value ) accounts for 90 deg rotation of the Physician screen
            int newValue = ( ( ( techMax - value ) * docRange ) / techRange ) + docMin;
            docWindow->ui.liveGraphicsView->horizontalScrollBar()->setValue( newValue );

            // prevent auxMon panning during zoom
            if( !isZooming )
            {
                newValue = ( ( value * auxRange ) / techRange ) + auxMin;
                auxMon->setLiveGraphicsViewVerticalScrollBar( newValue );
            }
        }
    }
}

/*
 * on_zoomResetPushButton_clicked
 */
void frontend::on_zoomResetPushButton_clicked()
{
    centerLiveGraphicsView();
    ui.zoomSlider->setValue( ui.zoomSlider->minimum() );
    LOG( INFO, "Zoom reset" )
}

/*
 * on_zoomSlider_sliderPressed
 */
void frontend::on_zoomSlider_sliderPressed()
{
    isZooming = true;
    LOG( INFO, "Zoom slider pressed" )
}

/*
 * on_zoomSlider_sliderReleased
 *
 * The zoomFactor logged may not be the final factor it the slider is
 * moved quickly, especially if it is dragged to the minimum.
 */
void frontend::on_zoomSlider_sliderReleased()
{
    isZooming = false;
    LOG( INFO, QString( "Zoom slider released - %1" ).arg( zoomFactorText ) )
}

/*
 * on_physicianPreviewButton_toggled
 */
void frontend::on_physicianPreviewButton_toggled( bool checked )
{
    docWindow->showPreview( checked );
    isPhysicianPreviewDisplayed = checked;
}

/*
 * handleDisplayingCapture
 */
void frontend::handleDisplayingCapture()
{
    if( !isImageCaptureLoaded )
    {
        isImageCaptureLoaded = true;
    }

    if( isRecordingClip )
    {
        ui.recordingLabel->hide();
        docWindow->ui.recordingLabel->hide();
        auxMon->setText( AuxMonitor::Recording, false );
    }

    ui.liveViewPushButton->show();
    ui.reviewWidget->disableClipSelection();
//    ui.loopMaskLabel->show();
    ui.measureModePushButton->show(); // Allow access to Measurement during capture review
    ui.measureModePushButton->setEnabled( true );
    configureDisplayForReview();
}

/*
 * configureDisplayForReview
 *
 * Configure the UI for the Review State by disabling
 * appropriate controls, changing the mouse cursor,
 * and updating or hiding indicators.
 */
void frontend::configureDisplayForReview()
{
    // Display a normal arrow over the sector when reviewing
    ui.liveGraphicsView->setToolTip( "" );

    disableCaptureButtons();
    hideCatheterView();
    docWindow->ui.timeFieldLabel->hide();
    auxMon->setText( AuxMonitor::TimeField, false );
    ui.scanSyncButton->setEnabled( false );
    ui.displayOptionsButton->setDisabled( true );
    ui.deviceSelectButton->setDisabled( true );
    ui.GroupBoxBandC->setDisabled( true );
    advView->setReviewState();
    ui.advancedViewButton->setDisabled( true );
    ui.recordLoopButton->setDisabled( true );
    viewOption->disableButtons();
    ui.liveViewPushButton->show();
    ui.annotateImagePushButton->setDisabled( true );
    ui.imagingDepthWidget->enableControls( false );
    ui.timeFieldLabel_mini->hide();

    // Turn off zooming
    on_zoomResetPushButton_clicked();

    if( isPhysicianPreviewDisplayed )
    {
        ui.physicianPreviewButton->setChecked( false );
    }
    // disable the button for now, make sure to re-enable in on_liveViewPushButton_clicked() and closePlayback()
    ui.physicianPreviewButton->setDisabled( true );

    m_scene->hideAnnotations();
    docWindow->configureDisplayForReview();
    auxMon->configureDisplayForReview();
    ui.deviceFieldLabel->setStyleSheet( "QLabel { font: 24pt DinPRO-Medium; color: yellow; }" );
    ui.label_live->setStyleSheet( "QLabel { color: yellow; font: 24pt DINPro-medium;}" );
}

/*
 * on_liveViewPushButton_clicked
 *
 * Restore the UI to the Live View state by enabling
 * appropriate controls, resetting the mouse cursor,
 * and displaying appropriate indicators.
 */
void frontend::on_liveViewPushButton_clicked()
{
    LOG( INFO, "Live View button clicked" )
    ui.liveGraphicsView->setToolTip( defaultSceneToolTip );
    setSceneCursor( QCursor( Qt::OpenHandCursor ) );

    ui.liveViewPushButton->hide();
//    ui.measureModePushButton->hide(); // Hide the Measurement button.
    setMeasurementMode( false ); // Dismiss measurement groupbox and overlays if they remain.

    if( isImageCaptureLoaded )
    {
        m_scene->dismissReviewImages();
    }

    if( isLoopLoaded )
    {
        isLoopLoaded = false;
        closePlayback();
        m_scene->dismissReviewImages();
    }

    if( isRecordingClip )
    {
        ui.recordingLabel->show();
        docWindow->ui.recordingLabel->show();
        auxMon->setText( AuxMonitor::Recording, true );
    }
    else
    {
        ui.reviewWidget->enableClipSelection();
//        ui.loopMaskLabel->hide();
    }

    enableCaptureButtons();

    docWindow->ui.timeFieldLabel->show();
    auxMon->setText( AuxMonitor::TimeField, true );

    ui.displayOptionsButton->setEnabled( true );
    ui.deviceSelectButton->setEnabled( true );
    ui.GroupBoxBandC->setEnabled( true );
    advView->setLiveState();
    ui.advancedViewButton->setEnabled( true );
    viewOption->enableButtons();
    ui.annotateImagePushButton->setEnabled( true );
    ui.timeFieldLabel_mini->show();

    configureControlsForCurrentDevice();

    if( isAnnotateOn )
    {
        ui.scanSyncButton->setDisabled( true );
    }

    showCatheterView();

    /*
     * Make sure capture buttons stay disabled if space is low
     */
    if( disableCapturing )
    {
        disableCaptureButtons();
    }

    /*
     * reviewStateEnded() must be called before on_zoomResetPushButton_clicked() because zoom reset
     * depends on the review state.
     */
    ui.reviewWidget->reviewStateEnded();
    on_zoomResetPushButton_clicked();

    m_scene->showAnnotations();
    docWindow->configureDisplayForLiveView();
    auxMon->configureDisplayForLiveView();
    ui.physicianPreviewButton->setEnabled( true ); // re-enable this button

    deviceSettings &dev = deviceSettings::Instance();
    ui.deviceFieldLabel->setText( dev.current()->getDeviceName() );
    ui.deviceFieldLabel->setStyleSheet( origDeviceLabelStyleSheet );
    ui.label_live->setStyleSheet( origLiveQLabelStyleSheet );
}

/*
 * hideLiveViewButton  // TBD: need better name, could be more actions that we need to take when dimissing images
 */
void frontend::hideLiveViewButton()
{
    if( isImageCaptureLoaded )
    {
        isImageCaptureLoaded = false;
    }

    on_liveViewPushButton_clicked();
}

/*
 * on_autoAdjustBrightnesscontrastButton_clicked
 */
void frontend::on_autoAdjustBrightnessContrastButton_clicked()
{
    LOG( INFO, "Auto-adjust brightness and contrast clicked" )
    emit autoAdjustBrightnessAndContrast();
    auto* sm = SignalModel::instance();
    sm->setWhiteLevel(0);
    sm->setBlackLevel(0);
    ui.horizontalSliderBrigtness->setValue(0);
    ui.horizontalSliderContrast->setValue(0);
}

/*
 * on_captureImageButton_clicked
 *
 * Grab the decorated display from the Physician screen and send the image
 * to the capturing system.
 */
void frontend::on_captureImageButton_clicked()
{
    TIME_THIS_SCOPE( frontend_on_captureImageButton_clicked );

    // tag the images as "img-001, img-002, ..."
    currImgNumber++;
    QString tag = QString( "%1%2" ).arg( ImagePrefix ).arg( currImgNumber, 3, 10, QLatin1Char( '0' ) );
    LOG1(tag);
    QRect rectangle = ui.liveGraphicsView->rect();
//    rectangle.setWidth(1440);
//    rectangle.setHeight(1440);
    qDebug() << __FUNCTION__ << ": width=" << rectangle.width() << ", height=" << rectangle.height();
    QImage p = ui.liveGraphicsView->grab(rectangle).toImage();
    m_scene->captureDi( p, tag );
}

/*
 * on_annotateImagePushButton_clicked
 *
 * Allow drawing on the image by the Technician. This will be displayed on
 * both screens.
 */
void frontend::on_annotateImagePushButton_clicked()
{
    isAnnotateOn = !isAnnotateOn;

    QColor currColor = QColor( Qt::yellow ).lighter( 150 );

    m_scene->setAnnotateMode( isAnnotateOn, currColor );

    if( isAnnotateOn )
    {
        ui.scanSyncButton->setDisabled( true );
        ui.liveGraphicsView->setToolTip( "" );
        setSceneCursor( QCursor( Qt::CrossCursor ) );  // TBD: pencil?
        LOG( INFO, "Annotate Mode: start" )
    }
    else
    {
        ui.scanSyncButton->setEnabled(false);
        if( !isZoomModeOn )
        {
            ui.liveGraphicsView->setToolTip( defaultSceneToolTip );
        }
        setSceneCursor( QCursor( Qt::OpenHandCursor ) );
        LOG( INFO, "Annotate Mode: stop" )
    }
}


/*
 * handleScreenChanges
 */
void frontend::handleScreenChanges()
{
    hideDisplays();
    createDisplays();
    testDisplays();
}

/*
 * handleBadMonitorConfig
 */
void frontend::handleBadMonitorConfig()
{
    captureMouse( false );
    hideDisplays();

    const auto& rect = wmgr->getDefaultDisplayGeometry();
    qDebug() << __FUNCTION__ << ", x=" << rect.x() << ", y=" << rect.y();
    qDebug() << __FUNCTION__ << ", w=" << rect.width() << ", h=" << rect.height();

    this->setGeometry( rect );
//    this->showFullScreen();//show(); //lcv this->showFullScreen();
    wmgr->showInfoMessage( this->parentWidget() );
    captureMouse( true );
}

/*
 * hideDisplays
 */
void frontend::hideDisplays()
{
    this->hide();
    if( auxMon )
    {
        auxMon->hide();
    }
    if( docWindow )
    {
        docWindow->hide();
    }
}

/*
 * createDisplays
 */
void frontend::createDisplays()
{
    // create displays even if they aren't going to be driven
    if( !docWindow )
    {
        docWindow = new docscreen( this );
        docWindow->hide();
    }
    if( !auxMon )
    {
        auxMon = new AuxMonitor( this );
        auxMon->hide();
    }

    this->hide();
    const auto& rect = wmgr->getTechnicianDisplayGeometry();
    qDebug() << __FUNCTION__ << ", x=" << rect.x() << ", y=" << rect.y();
    qDebug() << __FUNCTION__ << ", w=" << rect.width() << ", h=" << rect.height();
    this->setGeometry( rect );
//    showFullScreen(); //lcv this->showFullScreen(); show();


    docWindow->hide();
    if( !wmgr->getPhysicianDisplayGeometry().isNull() )
    {
        docWindow->setGeometry( wmgr->getPhysicianDisplayGeometry() );
//        docWindow->showFullScreen();//docWindow->show(); //lcv docWindow->showFullScreen();
    }

    if( wmgr->isAuxMonPresent() )
    {
        /*
         * This sequence is important to ensure the proper geometry is displayed on the Aux Monitor
         * and that the matrix is set properly, which is absolutely necessary for Zoom.
         */
//lcv disable aux monitor here
//        auxMon->show();
//        auxMon->setGeometry( wmgr->getAuxilliaryDisplayGeometry() );
//        auxViewMatrix = auxMon->getMatrix();
//        auxMon->showFullScreen();
    }
}

/*
 * testDisplays
 */
void frontend::testDisplays()
{
    bool checksAreGood = true;
    // compare frontend displays with WindowManager
    if( this->geometry() != wmgr->getTechnicianDisplayGeometry() )
    {
        checksAreGood = false;
    }
    if( docWindow->geometry() != wmgr->getPhysicianDisplayGeometry() )
    {
        checksAreGood = false;
    }
    if( auxMon->isHidden() && wmgr->isAuxMonPresent() )
    {
        checksAreGood = false;
    }
    if( wmgr->isAuxMonPresent() )
    {
        if( auxMon->geometry() != wmgr->getAuxilliaryDisplayGeometry() )
        {
            checksAreGood = false;
        }
    }
    else
    {
        if( !auxMon->isHidden() )
        {
            checksAreGood = false;
        }
    }

    if( checksAreGood )
    {
        wmgr->hideInfoMessage();
    }
//    else //lcv
//    {
//        handleScreenChanges();
//    }
}

/*
 * handleDaqReset()
 *
 * Handle the resetDaq call from HighSpeedDAQ. This is necessary to call only when a monitor
 * connection/disconnection causes the DAQ to hang up.
 */
void frontend::handleDaqReset()
{
    LOG( INFO, "DAQ Reset requested." )

    // Temporarily pause data while we start
    stopDataCapture();
    emit recordBackgroundData( false );
    shutdownHardware();

    // Restore updates
    m_scene->clearImages();

    startDaq();

    // turn on background recording (if enabled) and event log processing
    emit recordBackgroundData( true );
    emit forwardTurnDiodeOn();
    startDataCapture(); // Paused at the start of the wizard, so restart for rejected or accepted wizard state.
    configureControlsForCurrentDevice();
}
/*
 * setupDeviceForSledSupport
 */
void frontend::setupDeviceForSledSupport()
{
    qDebug() << "**** setupDevice";
    emit updateDeviceForSledSupport();
}

/*
 * shut down and bring up with new device speed
 */
void frontend::changeDeviceSpeed(int revsPerMin, int aLines )
{
    //qDebug() << "**** changeSpeed";
    LOG1(revsPerMin)
    deviceSettings &dev = deviceSettings::Instance();
//lcv    dev.current()->setLinesPerRevolution( aLines );
//    dev.current()->setRevolutionsPerMin( revsPerMin );
//    LOG2(revsPerMin, aLines);

//    lastDirCCW = dev.current()->getRotation();
    stopDataCapture();
    shutdownHardware();
    startDaq();
    emit forwardTurnDiodeOn();
    startDataCapture();
    setupDeviceForSledSupport();
}


void frontend::on_EgineeringButton_toggled(bool checked)
{
    m_ec->setViewPosition(ui.capturesGroupBox->x());
    m_ec->showOrHideView(checked);
}

void frontend::hideDecoration(void)
{
    if(ui.measureModePushButton->isChecked())
    {
        ui.measureModePushButton->clicked();
    }
}


void frontend::on_pushButtonLogo_clicked()
{
    if(m_formL300){
        //TODO synch depth
        const auto& sm = SignalModel::instance();
        const int* depth = sm->getImagingDepth_S();
        qDebug() << __FUNCTION__ << ": depth=" << *depth;
        m_formL300->setDepth(*depth);
        m_formL300->showFullScreen(); //lcv m_formL300->showFullScreen(); show();
    }
    if(m_mainScreen){
        m_mainScreen->showFullScreen();
    }
}
