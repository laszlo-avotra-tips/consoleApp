/*
 * frontend.h
 *
 *
 *
 * Author: Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#ifndef FRONTEND_H
#define FRONTEND_H

#include <QCursor>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>
#include "ui_frontend.h"
#include "docscreen.h"
#include "auxmonitor.h"
#include "Widgets/livescene.h"
#include "Widgets/advancedview.h"
#include "Widgets/viewoptions.h"
#include "Widgets/lagwizard.h"
#include "Widgets/caseinfowizard.h"
#include "Widgets/devicewizard.h"
#include "../../Common/GUI/transport.h"
#include "defaults.h"
#include <QThread>
#include <QDebug>
#include <QString>
#include "Utility/daqDataConsumer.h"
#include "session.h"
#include "Utility/directionTracker.h"
#include "buildflags.h"
#include "capturewidget.h"
#include "windowmanager.h"

#if ENABLE_COLORMAP_OPTIONS
#include "Widgets/curvesdialog.h"
#endif

const int mouseSamplingInterval(50); // msec

//class DAQ;
class IDAQ;
class EngineeringController;

class frontend : public QMainWindow
{
    Q_OBJECT

public:
    frontend(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    ~frontend();
    void init( void );
    void abortStartUp( void ) { appAborted = true; }
    void turnOffPhysicianScreen( void ) { docWindow->hide(); }
    int setupCase( bool isInitialSetup );

    void setIDAQ(IDAQ* object);

public slots:
    // methods to start/stop the data consumer thread
    void startDataCapture( void );
    void stopDataCapture( void );
    void startDaq( void );
    void pauseDaq( void );
    void resumeDaq( void );
    void stopDaq( void );
    void handleDaqReset();
    void setupDeviceForSledSupport();
    void changeDeviceSpeed(int, int);
    void dirButton( int );


    // These are sent from external objects to the frontend:
    // there is no feedback in general back to the sender, since
    // frontend is just presenting information opaquely. Frontend
    // errors are handled separately.
    void handleWarning( QString notice );
    void handleError( QString notice );  

    void handleDeviceChange();
    QDialog::DialogCode on_deviceSelectButton_clicked();
    void updateCaseInfo();
    void sendDaqLevel( QString level ) { emit forwardDaqLevel( level ); }
    void shutdownCleanup();
    void handleClipRecordingStopped();
    void handleLoopLoaded( QString loopFilename );
    void handleReviewDeviceName( QString str );
    void handleStatusText( QString status );
    void handleEndOfFile();
    void updateCatheterViewLabel();
    void handleManualLagAngle( double );
    void disableStorage( bool );
    void configureHardware();
    void shutdownHardware( void );
    void handleSendVideoDuration( int duration ) { clipLength_ms = duration; }
    void handleShowCurrentDeviceLabel();
    void handleScreenChanges();
    void handleBadMonitorConfig();
    void enableDisableMeasurementForCapture( int pixelsPerMm );

#if ENABLE_COLORMAP_OPTIONS
    void curvesDialogFinished();
    void curveMapChanged();
#endif

signals:
    void brightnessChange( int );
    void contrastChange( int );
    void autoAdjustBrightnessAndContrast( void );

    // Call before starting the data consumer to record data to disk. Call while
    // recording to turn recording on/off (i.e., changing devices, lag correction)
    void recordBackgroundData( bool );

    void tagEvent( QString );
    void forwardDaqLevel( QString );
    void forwardLaserDiodeStatus( bool );
    void checkLaserDiodeStatus();
    void forwardTurnDiodeOn();
    void forwardTurnDiodeOff();
#if ENABLE_SLED_SUPPORT_BOARD_TESTING
    void checkSledStatus();
#endif
    void announceClockingMode( int );
    void announceFirmwareVersions( QByteArray, QByteArray );
    void updateDeviceForSledSupport();

    void disableMouseRotateSector();
    void enableMouseRotateSector();
    void startClipRecording();
    void stopClipRecording();
    void setClipFilename( QString );
    void captureClipImages( QString );
    void setClipName( QString );
    void sendRepeatCheckBox( int );
    void forcePauseButtonOff();

    void directionOfRotation( directionTracker::Direction_T );
    void sendLagAngle( double );

#if ENABLE_ON_SCREEN_RULER
    void setSlidingPoint( int );
#endif

#if ENABLE_IPP_FFT_TUNING
    void magScaleValueChanged( int );
    void fftScaleValueChanged( int );
#endif
//    void setSledRotation( bool );
	void setDoPaint();

private:
    void setupScene( void );
    bool confirmExit( void );
    void playbackControlsVisible( bool state );
    void configureClock( void );
    void captureMouse( bool state );
    void enableCaptureButtons( void );
    void disableCaptureButtons( void );
    void closePlayback();
    void configureControlsForCurrentDevice( void );
    void hideCatheterView( void );
    void showCatheterView( void );
    void setSceneCursor( QCursor cursor );
    void centerLiveGraphicsView( void );
    void startDAQprepareView();

    IDAQ* idaq;
    bool appAborted;
    bool disableCapturing;
    bool isAnnotateOn;
    bool prevIsWaterfallVisible;

    QString origDeviceLabelStyleSheet;
    QString origLiveGroupBoxStyleSheet;

    bool isMeasureModeActive;
    bool measureModeAllowed;

    Ui::frontendClass ui;
    docscreen *docWindow;
    AuxMonitor *auxMon;

    advancedView *advView;
    viewOptions *viewOption;

    QWidget* m_ed;
    EngineeringController* m_ec;

    WindowManager *wmgr;

#if ENABLE_COLORMAP_OPTIONS
   	curvesDialog *curveDlg;
#endif

    liveScene *scene;

    lagWizard *lagHandler;
    caseInfoWizard *caseWizard;

    QTimer mouseCaptureTimer;
    QTimer clockTimer;
    QTimer storageSpaceTimer;
    QTimer preventFastRecordingsTimer;
    QTimer hideMouseTimer;

    bool preventFastRecordings;
    bool isPhysicianPreviewDisplayed;

    Session session;

#if ENABLE_SINGLE_STREAM_RECORDING
    bool      isShuttingDown;
#endif
    bool      isImageCaptureLoaded;

    bool      isLoopLoaded;
    bool      isClipPlaying;
    bool      isRecordingClip;
    int       currClipNumber;
    int       currImgNumber;
    QString   strClipNumber;
    QTime     clipTimer;
    int       clipLength_ms;
    QDateTime clipTimestamp;
    QString   playbackClipName;
    QString   clipCatheterView;

    QMatrix techViewMatrix;
    QMatrix docViewMatrix;
    QMatrix auxViewMatrix;

    bool    isZoomModeOn;
    bool    isZooming;
    QString zoomFactorText;
    QString defaultSceneToolTip;
    float   defaultZoomFactor;
    QRect   techMonRect;
    bool 	lastDirCCW;

public:
    DaqDataConsumer *consumer;

private slots:
    void handlePlayButton_clicked();
    void handlePauseButton_clicked();
    void on_recordLoopButton_clicked();
    void on_caseDetailsButton_clicked();
    void on_scanSyncButton_clicked();
    void on_advancedViewButton_clicked();
    void on_displayOptionsButton_clicked();
    void on_endCaseButton_clicked();
    void mouseTimerExpiry();
    void clockTimerExpiry();
    void hideMouseTimerExpiry();
    void storageSpaceTimerExpiry();
    void reenableRecordLoopButtonExpiry();

#if ENABLE_IPP_FFT_TUNING
    void on_magSpinBox_valueChanged( int arg1 );
    void on_fftSpinBox_valueChanged( int arg1 );
#endif

#if ENABLE_COLORMAP_OPTIONS
    void on_contrastCurveButton_clicked();
    void populateColormapList( void );
    void on_colormapListWidget_doubleClicked( const QModelIndex &index );
#endif

#if ENABLE_VIDEO_CRF_QUALITY_TESTING
    void on_crfTestSpinBox_valueChanged(const QString &arg1);
#endif

    void setMeasurementMode( bool enable );
    void on_measureModePushButton_clicked();
    void on_saveMeasurementButton_clicked();

    void on_zoomSlider_valueChanged(int value);
    void handleTechViewHorizontalPan(int value);
    void handleTechViewVerticalPan(int value);
    void on_zoomResetPushButton_clicked();
    void on_zoomSlider_sliderPressed();
    void on_zoomSlider_sliderReleased();
    void on_physicianPreviewButton_toggled(bool checked);
    void handleDisplayingCapture();
    void configureDisplayForReview( void );
    void on_liveViewPushButton_clicked();
    void hideLiveViewButton();
    void on_autoAdjustBrightnessContrastButton_clicked();
    void on_captureImageButton_clicked();
    void on_annotateImagePushButton_clicked();
    void on_directionPushButton_clicked();

#if ENABLE_ON_SCREEN_RULER
    void on_rulerSlidingPointSpinbox_valueChanged( int val );
#endif

    void createDisplays();
    void hideDisplays();
    void testDisplays();

    void on_EgineeringButton_toggled(bool checked);

protected:
     void closeEvent( QCloseEvent *event );
     bool eventFilter( QObject *watched, QEvent *event );
     void keyPressEvent( QKeyEvent *event );
};

#endif // FRONTEND_H
