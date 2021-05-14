/*
 * sectorItem.cpp
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
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>
#include <math.h>
#include "defaults.h"
#include "sectoritem.h"
#include "livescene.h"
#include "profiler.h"
//#include "deviceSettings.h"
#include "trigLookupTable.h"
#include "Utility/userSettings.h"
#include <QTime>
#include "depthsetting.h"
#include <Backend/interfacesupport.h>
#include "rotationIndicatorFactory.h"
#include "signalmodel.h"


//const QColor AggressiveSpinColor = QColor( 237, 237, 130 ).darker( 200 ); // yellow-ish
//const QColor PassiveSpinColor    = QColor( 70, 234, 242 ).darker( 200 );  // light blue-ish

namespace{
// Math defines
const double pi(3.1415);
const double degToRad(pi/180.0);
}

/*
 * Constructor
 */
sectorItem::sectorItem( QGraphicsItem *parent )
    : QGraphicsPixmapItem( parent ),CrossOverAngleChange_rad(3 * float( pi / 2))
{
//	qDebug() << ">>>>>> 1";
    currDirection = directionTracker::Stopped;

    status                   = 1;
    displayRotationAngle_deg = 0.0;
    newRotation_deg          = 0.0;
    rotating                 = false;
    allowRotation            = true;
    sectorShouldPaint        = false;

    twoLinesValid     = false;
    oldLineIsValid    = false;
    fullRotationFlag  = false;
    lastAngle_deg     = 0;
    currentAngle_deg  = 0;

    reticleBrightness           = DefaultReticleBrightness;
    laserIndicatorBrightness    = DefaultLaserIndicatorBrightness;
    oldReticleBrightness        = DefaultReticleBrightness;
    oldLaserIndicatorBrightness = DefaultLaserIndicatorBrightness;

    /*
     * Set up our drawing surface
     */
    sectorImage = new QImage( sectorSize, QImage::Format_Indexed8 );
    if( !sectorImage )
    {
        status = 0;
        return;
    }

    painter = new QPainter();
    if( !painter )
    {
        status = 0;
        return;
    }

    // Store values that do not change over the life of the sector
    centerX  = sectorImage->width()  / 2;
    centerY  = sectorImage->height() / 2;
    secWidth = sectorImage->width();

    sectorImage->fill( 0x00 ); // Black background

    // Set to working defaults
    internalImagingMask_px = 0;
    catheterRadius_px      = 0;
    linesPerRevolution     = 1024;  // Default for constructing. Any device change will cause this to be updated

    // rendering
    interpDirection  = 1;
//    interpMultiplier = 1.0;

    timestamp = 0;
    currDirection = directionTracker::Stopped;

    isPlayback = false;

    average.reset( RotaryAverageWidth, linesPerRevolution );
    angleInt.setLimit( AngleIntLimit );
    lastAngle_cnt = 0;
    sectorDecoratedImage = sectorImage->copy();

    isVideoOnly = false;
}

/*
 * Destructor
 */
sectorItem::~sectorItem()
{
//	qDebug() << ">>>>>> 2";
    if( sectorImage )
    {
        delete sectorImage;
    }
}

/*
 * mousePressEvent
 *
 * If a mouse click occurs within the sector, start the rotation
 * state. Drags while in this state will rotate the sector item.
 */
void sectorItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
//	qDebug() << ">>>>>> 3";
    // Already in rotating state, nop
    if( rotating )
    {
        return;
    }

    // Left button only
    if( !rotating && ( event->button() == Qt::LeftButton ) )
    {
        rotating = true;
    }
}

/*
 * deviceChanged
 *
 * The user has changed the device being used. Update the device specific
 * values cached here to reflect that.
 */
void sectorItem::deviceChanged(void)
{
    deviceSettings &devSettings = deviceSettings::Instance();
    auto* smi = SignalModel::instance();
    depthSetting::Instance();

//    internalImagingMask_px     = devSettings.current()->getInternalImagingMask_px();
    internalImagingMask_px     = *(smi->getInternalImagingMask_px());

    catheterRadius_px     = devSettings.current()->getCatheterRadius_px();

    LOG1(devSettings.current()->getDeviceName())
    LOG1(devSettings.current()->getInternalImagingMask_px())
    LOG1(devSettings.current()->getCatheterRadius_px())
    LOG1(devSettings.current()->getCatheterRadius_um())
    LOG1(devSettings.current()->getPixelsPerUm())

//    linesPerRevolution    = devSettings.current()->getLinesPerRevolution();
    linesPerRevolution = 1024;
    LOG2(linesPerRevolution, smi->linesPerRevolution())

    // Deep View disabled when selecting a new device
    currentDepth_mm       = devSettings.current()->getImagingDepth_mm();
    currentAlineLength_px = devSettings.current()->getALineLength_px();
//    currentDepth_mm       = *(smi->getStandardDepth_mm());
//    currentAlineLength_px = *(smi->getALineLength_px());
    LOG1(devSettings.current()->getImagingDepth_mm())
    LOG1(devSettings.current()->getALineLength_px())

    average.reset( RotaryAverageWidth, linesPerRevolution );
    unwinder.reset();

    // clean up the display for the new device
    clearImage();
    displayRotationAngle_deg = 0.0;
    rotateSector( displayRotationAngle_deg );
}

void sectorItem::setReticleBrightness(int value)
{
    reticleBrightness = value;
    sectorShouldPaint = true;
}

/*
 * computeAngleForPosition
 *
 * Given the mouse position within the sector, compute the rotation angle from
 * the original position so that we can properly transform the sector image.
 */
double sectorItem::computeAngleForPosition ( QPointF position )
{
//	qDebug() << ">>>>>> 5";
    // Find the deviation from the center of the image
    double x = position.x() - pixmap().width()/2;
    double y = position.y() - pixmap().height()/2;

    // Compute the angle
    return( atan2( x, y ) );
}

/*
 * mouseMoveEvent
 *
 * Translate the motion of the mouse within the sector to actual
 * rotation.
 */
void sectorItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
//	qDebug() << ">>>>>> 6";
    if( rotating )
    {
        QPointF startPosition = mapFromScene( event->buttonDownScenePos( Qt::LeftButton ) );

        double oldAngle_deg = ( 180.0 * computeAngleForPosition( startPosition ) / pi );
        double newAngle_deg = ( 180.0 * computeAngleForPosition( event->pos() )  / pi );

        newRotation_deg = displayRotationAngle_deg + oldAngle_deg - newAngle_deg;

        /*
         * Clamp the rotation to the unit circle.  Losing the fractional
         * part of the rotation is acceptable. Keeping the angle on the unit
         * circle is required for the GPU processing of the video frame.
         */
        if( newRotation_deg < 0 )
        {
            newRotation_deg += 360.0;
        }
        if( newRotation_deg > 360.0 )
        {
            newRotation_deg = int(newRotation_deg) % 360;
        }
//        LOG1(newRotation_deg);
        SignalModel::instance()->setDisplayAngle(float(newRotation_deg));
//        rotateSector( newRotation_deg );
    }
}

/*
 * mouseReleaseEvent
 *
 * Done with the rotation event, leave the rotation state and
 * store the new value.
 */
void sectorItem::mouseReleaseEvent( QGraphicsSceneMouseEvent * )
{
//	qDebug() << ">>>>>> 7";
    if( rotating )
    {
        displayRotationAngle_deg = newRotation_deg;
        rotating = false;
    }
}

/*
 * rotateSector
 *
 * Do the common operations necessary to rotate the display of the
 * sector.
 */
void sectorItem::rotateSector( double angle_deg )
{
//	qDebug() << ">>>>>> 8";
    QTransform matrix;

    // Locate the center of the image
    QPointF center = boundingRect().center();

    // These offsets depend on the anti-aliasing mode in use
    double yTranslation = center.y() + 0.5;
    double xTranslation = center.x() + 0.5;

    // Clamp between 0 and 360 degrees
    if( angle_deg < 0.0 )
    {
        angle_deg += 360.0;
    }
    if( angle_deg > 360.0 )
    {
        angle_deg -= 360.0;
    }

    // rotate and scale around the center of the item
    matrix.translate( xTranslation, yTranslation );
    matrix.rotate( angle_deg );
    matrix.translate( -xTranslation, -yTranslation );
    setTransform( matrix, false );
}

QImage *sectorItem::getSectorImage() const
{
    return sectorImage;
}

void sectorItem::setSectorImage(QImage *value)
{
    sectorImage = value;
}

/*
 * addFrame
 *
 * Main client call. Adds a frame of OCT data to the image buffer, using
 * the provided angle to transform
 */
void sectorItem::addFrame( QSharedPointer<scanframe> &data )
{
    // Copy the image into the display buffer
    memcpy( sectorImage->bits(), data->m_dispData->data(), SectorWidth_px * SectorHeight_px );
    sectorShouldPaint = true;
}

/*
 * paintSector
 *
 * Do final markup on the sector image and convert it to a QPixmap to make
 * ready for display. This function draws all the overlay elements onto
 * the sector, such as the reticle, the aiming line, and the unwind indicator.
 */
void sectorItem::paintSector ( bool force )
{
    // no change in the data
    if( !sectorShouldPaint && !force )
    {
        return;
    }

    sectorShouldPaint = false;

    QPixmap tmpPixmap = QPixmap::fromImage( *(sectorImage) );
    painter->begin( &tmpPixmap );
    painter->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );

    QPen laserReferencePen = QPen( QColor( 50, 50, 255, laserIndicatorBrightness ), 3, Qt::SolidLine, Qt::RoundCap );

    if( !isUnwinding() )
    {
        painter->setPen( laserReferencePen );
    }
    else
    {
        overlayUnwrapIndicator( getRemainingUnwind() );

        // fatten up the line to make it more visible
        laserReferencePen.setWidth( 4 );
        painter->setPen( laserReferencePen );
    }

    // Update our reference line position
    int x1 = sectorImage->width() / 2;
    int y1 = sectorImage->height() / 2;

    /*
     * Draw the catheter center mask circle.
     * - get the starting point for the catheter edge from depthSetting
     * - get the grayscale color of the catheter center mask
     * - draw the catheter center mask ellipse
     */
    depthSetting &depth = depthSetting::Instance();
    int catheterEdgePosition = depth.getCatheterEdgePosition();
    QBrush cathEdgeBrush( Qt::black, Qt::SolidPattern );
    QColor reticleColor( 50, 50, 255, getReticleBrightness() );
    QPen   reticlePen( QPen( reticleColor, 3, Qt::SolidLine, Qt::RoundCap) );
    painter->setPen( reticlePen );
    painter->setBrush( cathEdgeBrush );
    painter->drawEllipse( QRect( QPoint( x1 - catheterEdgePosition, y1 - catheterEdgePosition ),
                                 QPoint( x1 + catheterEdgePosition, y1 + catheterEdgePosition ) ) );

    QPen directionPen = QPen( QColor( 0, 0, 0 ), 6, Qt::SolidLine, Qt::RoundCap );

    // Change the color of the direction indicators.  Hide them if stopped.
    // Directions for this indicator are always determined by viewing proximal to
    // distal( from the handle to the tip)
    switch( currDirection )
    {
    case directionTracker::CounterClockwise:
        directionPen.setColor( PassiveSpinColor );
        break;

    case directionTracker::Stopped: /* no change */
        // cathGrayscaleHsvH must be converted to RGB for setColor()
        directionPen.setColor( Qt::black );
        break;

    case directionTracker::Clockwise:
        directionPen.setColor( AggressiveSpinColor );
        break;
    }

    setPixmap( tmpPixmap );
    painter->end();
}

/*
 * overlayUnwrapIndicator()
 *
 * Draw an arc on the sector if the system considers the
 * catheter to be in an unwinding condition (not rotating at
 * the distal end). The arc length corresponds to the remaining
 * angle to unwind.
 */
void sectorItem::overlayUnwrapIndicator( double angle )
{
//	qDebug() << ">>>>>> 12";
    double startAngle_deg = 360.0 - currentAngle_deg;
    userSettings &settings = userSettings::Instance();
    double unwindAmount( settings.lagAngle() );

    // fade the unwind indicator in and out.  It will be at the color's defined
    // full-scale alpha value at x% of the programmed lag correction
    const double MaxAngle_deg = unwindAmount * double(UnwrapMaxOpacityAngle_deg);
    double alphaPercent = unwinder.getUnwoundAngle() / MaxAngle_deg;

    // do not draw any indicator for small amounts of unwinding.  This prevents the
    // indicator from popping onto the screen for small amounts of backlash/whip
    // when stopping rotation (especially with the manual HHR)
    if( unwinder.getUnwoundAngle() < double(UnwrapMinOpacityAngle_deg) )
    {
        alphaPercent = 0.0;
    }
    else if( unwinder.getUnwoundAngle() > MaxAngle_deg )
    {
        alphaPercent = 1.0;
    }
    // else leave the value unchanged

    QColor arcColor( 0, 255, 0, int( 200 * alphaPercent ) ); // Green, semi-transparent

    const int PenWidth = 6;
    painter->setPen(QPen(arcColor, PenWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush(arcColor, Qt::SolidPattern));
    painter->drawArc( QRectF( 0.0, 0.0, sectorImage->width() - PenWidth, sectorImage->height() - PenWidth ),
                      int( startAngle_deg * 16.0 ), // converstion to Qt angle representation
                      getInitialUnwindDirection() * int( angle * 16.0 ) );
}

/*
 * freeze()
 *
 * Produce a QImage of the current sector, suitable for capturing and
 * return to the caller.
 */
QImage sectorItem::freeze ( void )
{
//	qDebug() << ">>>>>> 13";
    timestamp = QDateTime::currentDateTime().toUTC().toTime_t();

    // Copy the sector image to a local image that can be manipulated
    QImage tmp = sectorImage->copy();

    // Rotate about the center of the image; use +0.5 for rounding
    float yTranslation = ( float( tmp.height() ) / 2.0f ) + 0.5f;
    float xTranslation = ( float( tmp.width() )  / 2.0f ) + 0.5f;

    // Compute the bounding rectangle of the new circle, to crop
    // off the corners that will be rotated around.  Even though the image
    // looks round, it's really a rectangle!
    double clipPixels = sqrt( double( tmp.height() * tmp.height() ) +
                              double( tmp.width()  * tmp.width()  ) );
    clipPixels = ( clipPixels - tmp.width() ) / 2.0;

    QRect clipRect;
    clipRect.setX( int(clipPixels * fabs( sin( displayRotationAngle_deg * degToRad * 2.0 ) ) ) );
    clipRect.setY( clipRect.x() );
    clipRect.setWidth( tmp.width() );
    clipRect.setHeight( tmp.height() );

    // Rotate about the center of the image
    QTransform transform;
    transform.translate( double(xTranslation), double(yTranslation) );
    transform.rotate( displayRotationAngle_deg );
    transform.translate( double(-xTranslation), double(-yTranslation) );

    // rotate the image
    tmp = tmp.transformed( transform, Qt::SmoothTransformation );

    // return just the area that is defined by the clipping rectangle
    return( tmp.copy( clipRect ) );
}

/*
 * disableOverlays()
 *
 * Turn off laserIndicatorBrightness and reticleBrightness, and
 * store original values to be used by enableOverlays().
 */
void sectorItem::disableOverlays( void )
{
//	qDebug() << ">>>>>> 14";
    oldLaserIndicatorBrightness = laserIndicatorBrightness;
    oldReticleBrightness        = reticleBrightness;
    laserIndicatorBrightness    = 0;
    reticleBrightness           = 0;
}

/*
 * enableOverlays()
 *
 * Restore laserIndicatorBrightness and reticleBrightness to
 * original values stored by disableOverlays().
 */
void sectorItem::enableOverlays( void )
{
// 	qDebug() << ">>>>>> 15";
   laserIndicatorBrightness    = oldLaserIndicatorBrightness;
    reticleBrightness           = oldReticleBrightness;
}

/*
 * updateColorMap
 *
 * Change the color map used to display the 8-bit image
 */
void sectorItem::updateColorMap( QVector<QRgb> map )
{
//	qDebug() << ">>>>>> 16";
    sectorImage->setColorTable( map );
}

/*
 * constructor
 */
overlayItem::overlayItem( sectorItem *parent )
{
//	qDebug() << ">>>>>> 17";
    parentSector = parent;
    reticleBrightness = parentSector->getReticleBrightness();
    overlayPainter = new QPainter();
    overlayImage = new QImage( sectorSize, QImage::Format_ARGB32 );
    clearOverlay();
}

void overlayItem::clearOverlay()
{
    // Transparent background
//    LOG1("0")
    overlayImage->fill( 0 );
}


/*
 * destructor
 */
overlayItem::~overlayItem()
{
//	qDebug() << ">>>>>> 18";
    delete overlayPainter;
    delete overlayImage;
}

/*
 * render()
 *
 * Called to draw the overlays. It only needs to be called at device
 * selection.
 */
void overlayItem::render( void )
{
    auto& ds = deviceSettings::Instance();
    auto* dev = ds.current();
    static int overlayCount{0};
    LOG1(overlayCount) overlayCount++;

    if(dev->isBiDirectional()){
        auto rotationIndicatorOverlayItem = RotationIndicatorFactory::getRotationIndicator();
        auto interfaceSupport = InterfaceSupport::getInstance();
        int runningState{interfaceSupport->getLastRunningState()};

        if(runningState != 0){
            rotationIndicatorOverlayItem->show();
        }
    }

    depthSetting &depth = depthSetting::Instance();
    int numReticles = depth.getNumReticles();
    int pixelsPerMm = depth.getPixelsPerMm();
    int catheterEdgePosition = depth.getCatheterEdgePosition();

    // Create the target image
    QPixmap tmpPixmap = QPixmap::fromImage( *overlayImage );
    overlayPainter->begin( &tmpPixmap );

    // Draw reticle indicating depths
    QColor reticleColor( 50, 50, 255, parentSector->getReticleBrightness() );
    QPen  reticlePen( QPen( reticleColor, 3, Qt::SolidLine, Qt::RoundCap) );

    overlayPainter->setPen( reticlePen );
    overlayPainter->setBrush( ReticleBrush );

    int x1 = SectorWidth_px  / 2;
    int y1 = SectorHeight_px / 2;

    // Draw reticles
    int offset = catheterEdgePosition;

    for ( int i = 0; i < numReticles; i++ )
    {
        offset += pixelsPerMm;
        overlayPainter->drawEllipse( QRect( QPoint( x1 - offset, y1 - offset ),
                                     QPoint( x1 + offset, y1 + offset ) ) );
    }

    // draw tick marks at major cardinal positions
    {
        overlayPainter->save();

        // set the painter coordinates to the center
        overlayPainter->translate( x1, y1 );

        /*
         * Offset from the edge of the overlayImage where to begin drawing the tick marks.
         * This offset should be proportional to the internal imaging mask.
         */
        const int TickLength_px = 20;
        int edgeOffset_px = TickLength_px;

        const QLine TickMark( x1 - edgeOffset_px, 0, ( x1 - edgeOffset_px ) - TickLength_px, 0 );

        const int CardinalLineWidth_px = 10;
        const int OtherLineWidth_px    = CardinalLineWidth_px / 2;

        // 3, 6, 9, 12 o'clock markers
        overlayPainter->setPen( QPen( reticleColor.lighter( 125 ), CardinalLineWidth_px, Qt::SolidLine, Qt::FlatCap ) );
        for( int kr = 0; kr < 4; kr++ )
        {
            overlayPainter->drawLine( TickMark );
            overlayPainter->rotate( 90.0 );
        }

        // the other hour markers are smaller
        overlayPainter->setPen( QPen( reticleColor.lighter( 125 ), OtherLineWidth_px, Qt::SolidLine, Qt::FlatCap ) );
        for( int kr = 0; kr < 12; kr++ )
        {
            // only draw non-cardinal lines
            if( ( kr % 3 ) != 0 )
            {
                overlayPainter->drawLine( TickMark );
            }
            overlayPainter->rotate( 30.0 );
        }

        overlayPainter->restore();
    }

    setPixmap( tmpPixmap );
    overlayPainter->end();
}

