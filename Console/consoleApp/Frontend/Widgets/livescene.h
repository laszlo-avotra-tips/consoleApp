/*
 * livescene.h
 *
 * QGraphicsScene derived object that handles all live data presentation including
 * the sector and any indicators that go along with them.
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
#include "sectoritem.h"
#include "Utility/capturemachine.h"
#include "annotateoverlay.h"
#include "Widgets/areameasurementoverlay.h"

class RotationIndicatorOverlay2;

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

    bool getIsRotationIndicatorOverlayItemEnabled() const;
    void setIsRotationIndicatorOverlayItemEnabled(bool value);

    void setActive();
    void setPassive();
    void setIdle();

public slots:
    void captureDi( QImage decoratedImage, QString tagText );
    void generateClipInfo();
    void resetRotationCounter();
    void setWindOffset( bool enabled ) {
        sector->setWindOffset( enabled );
    }
    void setWindAngle( double angle ) {
        sector->setWindAngle( angle );
    }
    void setUnwind( bool enabled )
    {
        sector->setUnwind( enabled );
    }

    double getWindAngle( void ) {
        return sector->getWindAngle();
    }

    void handleDeviceChange(void) {
        sector->deviceChanged();
    }

    void handleReticleBrightnessChanged();

    void handleLaserIndicatorBrightnessChanged( int value ) {
        sector->setLaserIndicatorBrightness( value );
    }

    void handleLagWizardStart( void )
    {
        sector->disableOverlays();
        overlays->setVisible( false );
    }

    void handleLagWizardStop( void )
    {
        sector->enableOverlays();
        overlays->setVisible( true );
    }

    void showMessage( QString );
    
    void showReview(const QImage &);

    void resetSector( void )
    {
        sector->reset();
    }

    void dismissReviewImages( void );
    void handleDisableMouseRotateSector() { mouseRotationEnabled = false; }
    void handleEnableMouseRotateSector() { mouseRotationEnabled = true; }

    // Low Speed Device only
    void updateDirectionOfRotation( directionTracker::Direction_T currDirection )
    {
        rotationDirection = currDirection;
        sector->setDirection( currDirection );
    }

    void clearSector()
    {
        sector->clearImage();
    }

    void resetIntegrationAngle( void )
    {
        sector->resetIntegrationAngle();
    }
	
	void setDoPaint()
	{
		doPaint = true;
	}

    void setClipForPlayback( QString name );
    void startPlayback( void );
    void pausePlayback(  );
    void advancePlayback( );
    void rewindPlayback( );

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
    void captureAll( QImage, QImage, QString, unsigned int, int, float );
    void clipCapture( QImage , QString, unsigned int );
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
    bool doPaint{false};
    const bool force{true};
    sectorItem *sector;
    overlayItem *overlays;
    float zoomFactor;
    bool reviewing;
    bool mouseRotationEnabled;
    bool isAnnotateMode;
    bool  isMeasureMode;
    int   cachedCalibrationScale;

    QGraphicsPixmapItem *reviewSector;

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

    AnnotateOverlay *annotateOverlayItem{nullptr};
    bool isAnnotateModeEnabled{false};

    AreaMeasurementOverlay *areaOverlayItem{nullptr};
    bool isMeasurementEnabled{false};

    RotationIndicatorOverlay2* rotationIndicatorOverlayItem{nullptr};
    bool isRotationIndicatorOverlayItemEnabled{true};

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};
