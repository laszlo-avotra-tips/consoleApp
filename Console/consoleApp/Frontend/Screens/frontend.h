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
#include "Widgets/livescene.h"
#include "Widgets/viewoptions.h"
#include "Widgets/lagwizard.h"
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
#include "windowmanager.h"
#include <memory>
#include <forml300.h>
#include "scanconversion.h"
#include <QTime>


const int mouseSamplingInterval(50); // msec

//class DAQ;
class IDAQ;
class EngineeringController;
class FormL300;
class MainScreen;

class frontend : public QWidget
{
    Q_OBJECT

public:
    frontend(QWidget *parent = nullptr);
    ~frontend();
    void init( void );
    void abortStartUp( void ) { appAborted = true; }
    void updateDeviceLabel();

    void setIDAQ(IDAQ* object);
    void showSpeed(bool isShown);

public slots:
    // methods to start/stop the data consumer thread
    void setMeasurementMode( bool enable );
    void startDataCapture( void );
    void stopDataCapture( void );
    void startDaq( void );
    void pauseDaq( void );
    void resumeDaq( void );
    void stopDaq( void );
    void handleDaqReset();
    void setupDeviceForSledSupport();
    void changeDeviceSpeed(int, int);


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
    void shutdownHardware( void );
    void handleSendVideoDuration( int duration ) { clipLength_ms = duration; }
    void handleShowCurrentDeviceLabel();
    void handleScreenChanges();
    void handleBadMonitorConfig();
    void enableDisableMeasurementForCapture( int pixelsPerMm );
    void updateSector(OCTFile::OctData_t*);
    void on_zoomSlider_valueChanged(int value);

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
    void checkSledStatus();
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

//    void setSledRotation( bool );
	void setDoPaint();

private:
    void setupScene( void );
    bool confirmExit( void );
    void playbackControlsVisible( bool state );
    void configureClock( void );
    void captureMouse(bool);
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

    QString origDeviceLabelStyleSheet;
    QString origLiveGroupBoxStyleSheet;
    QString origLiveQLabelStyleSheet;

    bool isMeasureModeActive;
    bool measureModeAllowed{true};

    Ui::frontendClass ui;
    viewOptions *viewOption;

    QWidget* m_ed;
    EngineeringController* m_ec;

    WindowManager *wmgr;

    liveScene *m_scene{nullptr};

    lagWizard *lagHandler;

    QTimer mouseCaptureTimer;
    QTimer clockTimer;
    QTimer storageSpaceTimer;
    QTimer preventFastRecordingsTimer;
    QTimer hideMouseTimer;

    bool preventFastRecordings;
    bool isPhysicianPreviewDisplayed;

    Session session;

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

#if ENABLE_VIDEO_CRF_QUALITY_TESTING
    void on_crfTestSpinBox_valueChanged(const QString &arg1);
#endif

    void on_measureModePushButton_clicked();

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

    void on_EgineeringButton_toggled(bool checked);
    void hideDecoration();

    void on_pushButtonLogo_clicked();

protected:
     void closeEvent( QCloseEvent *event );
     bool eventFilter( QObject *watched, QEvent *event );
     void keyPressEvent( QKeyEvent *event );

private:
     FormL300* m_formL300{nullptr};
     MainScreen* m_mainScreen{nullptr};
     ScanConversion *m_scanWorker{nullptr};
};

#endif // FRONTEND_H
