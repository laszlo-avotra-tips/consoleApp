/*
 * sectorItem.h
 *
 * The sector item is a QGraphicsPixmapItem that handles all the rendering
 * of the azimuthal OCT view. Clients update the view by sending addLine()
 * signals containing the desired data and angle. The item performs all
 * geometric conversions and renders the image. The item also handles
 * the display of all overlays, such as the depth reticle and the unwind
 * indicator.
 *
 * Author(s): Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger Inc.
 */

#pragma once

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QRgb>
#include <QSharedPointer>
#include <QVector>
#include "rotaryAverage.h"
#include "unwindMachine.h"
#include "defaults.h"
#include "buildflags.h"
#include "util.h"
#include "Integrator.h"
#include "scanLine.h"
#include "Utility/directionTracker.h"

/*
 * Constants and #defines
 */


/*
 * Class declarations
 */
class sectorItem : public QGraphicsPixmapItem
{

public:
    sectorItem(QGraphicsItem *parent = nullptr);
    ~sectorItem();

    void deviceChanged(void);
    void paintRadar( void );
    double getCurrentAngle( void )
    {
        return currentAngle_deg;
    }
    double getDisplayAngle(void)
    {
        return displayRotationAngle_deg;
    }
    void setDisplayAngle( double angle )
    {
        displayRotationAngle_deg = angle;
    }
    void setWindOffset( bool enabled )
    {
        unwinder.enableLargeUnwind( enabled );
    }
    void setWindAngle(double angle)
    {
        unwinder.setWindAngle( angle );
    }
    void setUnwind( bool enabled )
    {
        unwinder.setEnabled( enabled );
    }
    double getWindAngle( void )
    {
        return unwinder.getWindAngle();
    }
    void unwindUpdate(double angle)
    {
        unwinder.update( angle );
    }
    bool isUnwinding(void)
    {
        return( unwinder.isUnwinding() );
    }
    double getRemainingUnwind(void)
    {
        return( unwinder.getRemainingUnwind() );
    }
    double getWindOffset(void)
    {
        return( unwinder.getOffset() );
    }
    int getInitialUnwindDirection(void)
    {
        return( unwinder.getInitialDirection() );
    }
    bool fullRotationCompleted(void)
    {
        return( fullRotationFlag );
    }
    void clearRotationFlag(void)
    {
        fullRotationFlag = false;
    }
    void setReticleBrightness( int value )
    {
        reticleBrightness = value;
        sectorShouldPaint = true;
    }
    int getReticleBrightness( void )
    {
        return reticleBrightness;
    }
    void setLaserIndicatorBrightness( int value )
    {
        laserIndicatorBrightness = value;
        sectorShouldPaint = true;
    }
    void clearImage( void )
    {
        sectorImage->fill( 0xFF000000 );
        sectorShouldPaint = true;
    }
    void addFrame( QSharedPointer<scanframe> &data );
    void paintSector( bool force );
    QImage freeze( void );
    unsigned int getFrozenTimestamp( void )
    {
        return timestamp;
    }
    char *frameData()
    {
        sectorDecoratedImage = sectorImage->copy();
        return reinterpret_cast<char *>(sectorDecoratedImage.bits() );
    }
    int getStatus( void )
    {
        return status;
    }
    void setDirection( directionTracker::Direction_T dir )
    {
        currDirection = dir;
    }
    void setPlayback( bool state )
    {
        isPlayback = state;
    }

    /*
     * Reset the flags for valid line data. This disables interpolation between
     * the last line received and the next one that comes in.
     */
    void reset( void )
    {
        oldLineIsValid = false;
    }

    void disableOverlays( void );
    void enableOverlays( void );
    void resetIntegrationAngle( void )
    {
        angleInt.reset();
    }

    void setCurrentAlineLength_px( int val )
    {
        currentAlineLength_px = val;
    }
    void setCurrentDepth_mm( float val )
    {
        currentDepth_mm = val;
    }
    void setCatheterRadius_px( int val )
    {
        catheterRadius_px = val;
    }
    void updateColorMap( QVector<QRgb> map );
    void setVideoOnly() { isVideoOnly = true; }
#if ENABLE_ON_SCREEN_RULER
    void setSlidingPoint( int val )
    {
        slidingPoint = val;
    }
#endif

private:
    int status;
    void render(void);
    void overlayUnwrapIndicator( double angle );

    double computeAngleForPosition(QPointF position);
    double distanceToPoint(QPointF point);
    void rotateSector(double angle_deg);

    QImage *sectorImage;
    QImage sectorDecoratedImage;

    QPainter *painter;

    int centerX;
    int centerY;
    int secWidth;

#if ENABLE_ON_SCREEN_RULER
    int slidingPoint;
#endif

    unwindMachine unwinder;
    Integrator angleInt;

    rotaryAverage average;

    QByteArray oldLine;
    QByteArray newLine;

    bool oldLineIsValid;
    bool twoLinesValid;

    bool sectorShouldPaint;

    double currentAngle_deg;
    double lastAngle_deg;
    bool fullRotationFlag;

    bool rotating;
    bool allowRotation;
    double displayRotationAngle_deg;
    double newRotation_deg;

    int reticleBrightness;
    int laserIndicatorBrightness;
    int oldReticleBrightness;
    int oldLaserIndicatorBrightness;

    // Cached for speed. Changed when device changes (the parent scene
    // is notified via signal)
    int   internalImagingMask_px;
    int   catheterRadius_px;
    int   linesPerRevolution;
    int   currentAlineLength_px;
    float currentDepth_mm;

    // rendering
    int interpDirection;
//    float interpMultiplier;

    unsigned int timestamp;
    directionTracker::Direction_T currDirection;

    bool isPlayback;
    short lastAngle_cnt;

    bool isVideoOnly;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    // Integrator limit
    const int AngleIntLimit {720};
    // Minimum angular resolution we try to fill in when interpolating
    const double MinInterpolationAngle_rad { MININTERPOLATIONANGLE_RAD };
    // How many angle samples to average to produce current angle value
    const int RotaryAverageWidth { 4 };
    // Default sector size
    const QSize sectorSize {SectorHeight_px, SectorWidth_px };
    // Detect flip around 360 by a large instantaneous change in angle
    const float CrossOverAngleChange_rad;// {3 * float( pi / 2)};
    const float UnwrapMinOpacityAngle_deg {0.2f};
    // Constants for rendering various things
    const float UnwrapMaxOpacityAngle_deg {0.8f};

};

/*
 * overlayItem class
 */
class overlayItem : public QGraphicsPixmapItem
{
public:
    overlayItem( sectorItem *parent = nullptr );
    ~overlayItem();
    void render( void );
private:
    bool overlaysShouldPaint;
    QPainter   *overlayPainter;
    QImage     *overlayImage;
    sectorItem *parentSector;
    int reticleBrightness;

    const QSize sectorSize {SectorHeight_px, SectorWidth_px };
    // Reticle and indicator colors/pens/brushes
    const QBrush ReticleBrush {Qt::NoBrush};
};
