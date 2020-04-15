/*
 * livescene.h
 *
 * QGraphicsScene derived object that handles all live data presentation including
 * the sector and the waterfall and any indicators that go along with them.
 * 
 * This object also consumes the line data directly and adds it to the relevant
 * display objects.
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#pragma once

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include "waterfall.h"
#include "sectoritem.h"
//lcv #include "../../Common/GUI/videodecoderitem.h"
#include "Utility/capturemachine.h"
#include "buildflags.h"
#include "annotateoverlay.h"
#include "Widgets/areameasurementoverlay.h"

class liveScene : public QGraphicsScene
{
    Q_OBJECT

public:
    static const int ColorTableSize = 256;
    static const int SectorItemKey  = 1;

    liveScene( QObject *parent = nullptr );
    ~liveScene();
    void setZoomFactor( float factor ) { zoomFactor = factor; }
    void setMeasureModeArea( bool state, QColor color );
    void setAnnotateMode( bool state, QColor color );
    void applyClipInfoToBuffer( char *buffer );

    // grab the sector data for the OCT Loop vdideo encoding
    char *frameSample() { return( videoSector->frameData() ); }

    void  wfSample( char *buffer ) { wf->sample( buffer ); }
    void lockFrame() {
        frameLock.lock();
    }
    void unlockFrame() {
        frameLock.unlock();
    }
    QGraphicsPixmapItem *sectorHandle( void ) { return sector; }
    QImage* sectorImage() const {return sector->getSectorImage();}
    void hideAnnotations();
    void showAnnotations();

public slots:
    void addScanFrame( QSharedPointer<scanframe> &data );
    void capture( QImage decoratedImage, QString tagText );
    void captureClip( QString strIter );
    void generateClipInfo();
    void resetRotationCounter();
    void setWindOffset( bool enabled ) {
        sector->setWindOffset( enabled );
        videoSector->setWindOffset( enabled );
    }
    void setWindAngle( double angle ) {
        sector->setWindAngle( angle );
        videoSector->setWindAngle( angle );
    }
    void setUnwind( bool enabled )
    {
        sector->setUnwind( enabled );
        videoSector->setUnwind( enabled );
    }

    double getWindAngle( void ) {
        return sector->getWindAngle();
    }

    void handleDeviceChange(void) {
//		qDebug() << "**** livescene::handleDeviceChange()";
        sector->deviceChanged();
        videoSector->deviceChanged();
        wf->deviceChanged();
    }

    void handleReticleBrightnessChanged( int value ) {
        sector->setReticleBrightness( value );
    }

    void handleLaserIndicatorBrightnessChanged( int value ) {
        sector->setLaserIndicatorBrightness( value );
    }

    void handleLagWizardStart( void )
    {
        sector->disableOverlays();
        overlays->setVisible( false );
        wf->hide();
    }

    void handleLagWizardStop( void )
    {
        sector->enableOverlays();
        overlays->setVisible( true );
        wf->show();
    }

    void showMessage( QString );
    
    void showReview( const QImage &, const QImage & );

    void clearImages( void )
    {
        sector->clearImage();
        videoSector->clearImage();
        wf->clearImage();
        doPaint = true;
        refresh();
    }

    void resetSector( void )
    {
        sector->reset();
        videoSector->reset();
    }

    void dismissReviewImages( void );
    void handleDisableMouseRotateSector() { mouseRotationEnabled = false; }
    void handleEnableMouseRotateSector() { mouseRotationEnabled = true; }

    bool getWaterfallVisible( void ) { return isWaterfallVisible; }

    // Low Speed Device only
    void updateDirectionOfRotation( directionTracker::Direction_T currDirection )
    {
        rotationDirection = currDirection;
        sector->setDirection( currDirection );
    }

    void stopPlayback()
    {
//        clipPlayer->pause();
//        clipPlayer->hide();
    }

    void restartLoop()
    {
//        clipPlayer->seek( 0 );
//        clipPlayer->play();
    }

    void clearSector()
    {
        sector->clearImage();

        // video export do not indicate orientation but clear the image to prevent possible streaking
        videoSector->clearImage();
    }

    void resetIntegrationAngle( void )
    {
        sector->resetIntegrationAngle();
        videoSector->resetIntegrationAngle();
    }
	
	void setDoPaint()
	{
		doPaint = true;
		force = true;
	}

#if ENABLE_ON_SCREEN_RULER
    void setSlidingPoint( int val )
    {
        sector->setSlidingPoint( val );
    }
#endif

    void setClipForPlayback( QString name );
    void startPlayback( void );
    void pausePlayback(  );
    void advancePlayback( );
    void rewindPlayback( );
    void seekWithinClip( qint64 );

    void displayWaterfall( bool );
    void updateGrayScaleMap( QVector<unsigned char> map );
    void loadColormap( QString colormapFile );
    void loadColorModeGray();
    void loadColorModeSepia();
    void setCalibrationScale( int pixelsPerMm, float zoomFactor );
	void refresh();
	
signals:
    void fullRotation();
    void sendFileToKey( QString );
    void sendCaptureTag( QString );
    void sendStatusText( QString );
    void capture( QImage, QImage, QImage, QString, unsigned int, int, float );
    void clipCapture( QImage, QImage , QString, unsigned int );
    void updateCaptureCount();
    void updateClipCount();
    void sendWarning( QString );
    void sendError( QString );
    void endOfFile( );
    void clipLengthChanged( qint64 );
    void videoTick( qint64 );
    void showCurrentDeviceLabel();
    void sendDisplayAngle( float );
    void reviewImageDismissed();

    void measurementArea( int );
    void measurementLength( int );

private slots:
//    void refresh();

private:
    QVector<QRgb> grayScalePalette;
    QVector<QRgb> currColorMap;

    captureMachine capturer;
    QGraphicsTextItem *infoMessageItem;
    QTimer *refreshTimer;
    QTimer *infoRenderTimer;
    bool doPaint;
	bool force;
    waterfall  *wf;
    sectorItem *sector;
    sectorItem *videoSector;
    overlayItem *overlays;
    float zoomFactor;
    bool isWaterfallVisible;
    bool reviewing;
    bool mouseRotationEnabled;
    bool isAnnotateMode;
    bool  isMeasureMode;
    int   cachedCalibrationScale;

    QGraphicsPixmapItem *reviewSector;
    QGraphicsPixmapItem *reviewWaterfall;

//    videoDecoderItem *clipPlayer;
    QString           clipPath;

    QImage *infoImage;
    QImage  infoLogoImage;

    QImage  clockwiseRingImage;
    QImage  counterClockwiseRingImage;
    QImage  stoppedRingImage;
    QImage *pDirRingImage;
    int     dirRingImageHeight_px;
    directionTracker::Direction_T rotationDirection;

    QImage  activeIndicatorImage;
    QImage  passiveIndicatorImage;
    QImage  activeIndicatorRingImage;
    QImage  passiveIndicatorRingImage;

    char   *infoRenderBuffer;
    QMutex  infoRenderLock;
    QMutex  frameLock;

    AnnotateOverlay *annotateOverlayItem;
    bool isAnnotateModeEnabled;

    AreaMeasurementOverlay *areaOverlayItem;
    bool isMeasurementEnabled;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};
