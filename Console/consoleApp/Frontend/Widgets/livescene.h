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

struct IRotationIndicator;

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

    void lockFrame() {
        frameLock.lock();
    }
    void unlockFrame() {
        frameLock.unlock();
    }
    sectorItem *sectorHandle( void ) { return sector; }
    QImage* sectorImage() const {return sector->getSectorImage();}
    void hideAnnotations();
    void showAnnotations();

    bool getIsRotationIndicatorOverlayItemEnabled() const;
    void setIsRotationIndicatorOverlayItemEnabled(bool value);

    void setActive();
    void setPassive();
    void setIdle();
    void paintOverlay();
    void clearOverlay();

public slots:
    void captureDecoratedImage( QImage decoratedImage, QString tagText );
    void captureClip( QString strIter );
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

    void dismissReviewImages( void );
    void handleDisableMouseRotateSector() { mouseRotationEnabled = false; }
    void handleEnableMouseRotateSector() { mouseRotationEnabled = true; }
	
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
	
signals:
    void fullRotation();
    void sendFileToKey( QString );
    void sendStatusText( QString );
    void captureAll( QImage, QImage, QString, unsigned int, int, float );
    void clipCapture( QImage , QString, unsigned int );
    void sendWarning( QString );
    void sendError( QString );
    void endOfFile( );
    void clipLengthChanged( qint64 );
    void videoTick( qint64 );
    void showCurrentDeviceLabel();
    void reviewImageDismissed();

    void measurementArea( int );
    void measurementLength( int );

private slots:
    void refresh();

private:
    QVector<QRgb> grayScalePalette;
    QVector<QRgb> currColorMap;

    captureMachine capturer;
    QGraphicsTextItem *infoMessageItem;
    QTimer *refreshTimer;
    QTimer *infoRenderTimer;
    sectorItem *sector;
    overlayItem *overlays;
    float zoomFactor;
    bool reviewing;
    bool mouseRotationEnabled;
    bool isAnnotateMode;
    bool  isMeasureMode;
    int   cachedCalibrationScale;

    QGraphicsPixmapItem *reviewSector;

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

    IRotationIndicator* rotationIndicatorOverlayItem{nullptr};
    bool isRotationIndicatorOverlayItemEnabled{false};
    bool isTheMouseInTheCenter(QGraphicsSceneMouseEvent *event) const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};
