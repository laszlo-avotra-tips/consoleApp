/*
 * areameasurementoverlay.cpp
 *
 * Author: Dennis W. Jackson, Ryan F. Radjabi
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 */
#include <QPainter>
#include <QInputDialog>
#include <QDebug>
#include <QStaticText>
//#include "buildflags.h"
#include "../Console/buildflags.h"
#include "defaults.h"
#include "areameasurementoverlay.h"
#ifndef MEASUREMENT_APP
#include "depthsetting.h"
#endif
#include <qmath.h>

const int PointTolerance = 20;

#if !ENABLE_MEASUREMENT_PRECISION
const int MeasurementPrecision = 1;
#else
const int MeasurementPrecision = 2;
#endif

/*
 * Constructor
 */
AreaMeasurementOverlay::AreaMeasurementOverlay( QWidget * )
    : QGraphicsPixmapItem()
{
#ifndef MEASUREMENT_APP
    overlayPixmap = new QPixmap( SectorWidth_px, SectorHeight_px  );

    // Position the box near the bottom right corner with space for text.
    box = new QRect( overlayPixmap->width() - 280, overlayPixmap->height() - 180, 1, 1 );
#else
    overlayPixmap = new QPixmap( parent->geometry().width(), parent->geometry().height()  );
    box = new QRect( overlayPixmap->width() - 280, overlayPixmap->height() - 180, 1, 1 );
#endif
    setPixmap( *overlayPixmap );
    currentColor     = Qt::yellow;
    mouseIsDown      = false;
    allowReplace     = false;
    enableDrawMinMax = true;
    clickInBox       = false;
    boxDx = 0;
    boxDy = 0;

    centroid.isValid = false;

    replacementPointIndex = -1;
    currPxPerMm = 0.0f;
}

/*
 * Destructor
 */
AreaMeasurementOverlay::~AreaMeasurementOverlay()
{
    if( overlayPixmap )
    {
        delete overlayPixmap;
    }

    if( box )
    {
        delete box;
    }
}

/*
 * mousePressEvent
 *
 * only add a point on first press
 */
void AreaMeasurementOverlay::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // drag the box
    if( box->contains( event->pos().toPoint() ) )
    {
        clickInBox = true;
        boxCoordX = event->pos().toPoint().x();
        boxCoordY = event->pos().toPoint().y();
    }
    else
    {
        if( polygonPoints.isEmpty() ) // only add a point on first press
        {
            polygonPoints.append( event->pos().toPoint() );
        }

        // if within tolerance of a previous point, allow dragging and replace of that point.
        QPoint p1 = event->pos().toPoint();
        replacementPointIndex = pointsOverlap( &p1, &polygonPoints, PointTolerance );
        if( replacementPointIndex >= 0 )
        {
            allowReplace = true;
        }
        else
        {
            if( addControlPoint( event->pos().toPoint() ) )
            {
                QPoint point1 = event->pos().toPoint();
                replacementPointIndex = pointsOverlap( &point1, &polygonPoints, PointTolerance );
                if( replacementPointIndex >= 0 )
                {
                    allowReplace = true;
                }
            }
        }

        mouseIsDown = true;
    }
}

/*
 * mouseMoveEvent
 *
 * Don't allow the user to drag the mouse outside the sector view (unzoomed). This keeps the
 * calculations box from going out of view, as well as the control points.
 */
void AreaMeasurementOverlay::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    const int Top    = 0;
    const int Left   = 0;
    const int Right  = SectorWidth_px;
    const int Bottom = SectorHeight_px; // XXX we have to restrict the area measurement overlay to sector region for
                                        // unknown reasons.
    if( event->pos().toPoint().x() < Left )
    {
        event->setPos( QPointF( Left, event->pos().toPoint().y() ) );
    }
    if( event->pos().toPoint().y() < Top )
    {
        event->setPos( QPointF( event->pos().toPoint().x(), Top ) );
    }
    if( event->pos().toPoint().x() > Right )
    {
        event->setPos( QPointF( Right, event->pos().toPoint().y() ) );
    }
    if( event->pos().toPoint().y() > Bottom )
    {
        event->setPos( QPointF( event->pos().toPoint().x(), Bottom ) );
    }

    // Move the calculations box into view explicitly, even if the user is attempting to drag outside.
    if( clickInBox )
    {
        box->translate( event->pos().toPoint().x() - boxCoordX, event->pos().toPoint().y() - boxCoordY );
        boxCoordX = event->pos().toPoint().x();
        boxCoordY = event->pos().toPoint().y();

        if( box->left() < Left )
        {
            box->moveLeft( Left );
        }
        if( box->top() < Top )
        {
            box->moveTop( Top );
        }
        if( box->right() > Right )
        {
            box->moveRight( Right );
        }
        if( box->bottom() > Bottom )
        {
            box->moveBottom( Bottom );
        }
    }
    else
    {
        if( mouseIsDown && allowReplace )
        {
            // replace the point in this condition
            polygonPoints.replace( replacementPointIndex, event->pos().toPoint() );
        }
        else if( ( mouseIsDown && polygonPoints.size() <= 2 ) )
        {
            if( polygonPoints.size() == 1 )
            {
                polygonPoints.append( event->pos().toPoint() );
            }
            else
            {
                polygonPoints.replace( ( polygonPoints.size() - 1 ), event->pos().toPoint() );
            }
        }
    }
}

/*
 * mouseReleaseEvent
 *
 * add points on release
 */
void AreaMeasurementOverlay::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    mouseIsDown = false;

    // Don't duplicate the last point on mouse release.
    if( !clickInBox && event->pos().toPoint() != polygonPoints.last() )
    {
        // only add a point if replacement is disabled
        if( !allowReplace )
        {
            polygonPoints.append( event->pos().toPoint() );
        }
    }

    clickInBox = false;
    allowReplace = false;
}

/*
 * paint
 */
void AreaMeasurementOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    calculate();
    paintCalculationBox( painter );
    paintPolygon( painter );
    paintControlPoints( painter );
    paintMinMax( painter );
}

/**********************************
 * Polygon Calculation functions **
 **********************************/

/*
 * pointsOverlap
 *
 * Returns the index of the overlapping point in the polygon. If no points overlap, returns
 * -1. If multiple points overlap, it will return the index of the earliest added point in
 * the polygon.
 */
int AreaMeasurementOverlay::pointsOverlap(QPoint *p1, QPolygon *list, int tolerance)
{
    int retVal = -1; // return -1 if false, else return index of point that overlaps
    QRect r1( p1->x() - tolerance, p1->y() - tolerance, tolerance*2, tolerance*2 );

    for( int i = 0; i < list->size(); i++ )
    {
        if( r1.contains( list->at( i ) ) )
        {
            return i;
        }
    }

    return retVal;
}

/*
 * polygonToPoints
 *
 * Returns a new point list consisting of additional control points.
 */
QPolygon AreaMeasurementOverlay::polygonToPoints( QPolygon *list )
{
    QPolygon manyPoints;

    if( list->size() >= 2 )
    {
        for( int i = 0; i < list->size(); i++ )
        {
            QLineF seg;
            if( i == ( list->size() - 1 ) )
            {
                // This segment is from the last point in list to the first point that closes the polygon.
                seg = QLineF( list->point( i ), list->point( 0 ) );
            }
            else
            {
                seg = QLineF( list->point( i ), list->point( i + 1 ) );
            }

            const int segLen = int(seg.length());

            // add control point at end of unit vector originating from p1
            QLineF uv = seg.unitVector();

            QLineF l( list->point( i ), uv.p2() );

            while( l.length() < segLen )
            {
                seg.setP1( uv.p2() );
                uv = seg.unitVector();
                uv.setLength( PointTolerance );
                l.setP2( uv.p2() );

                manyPoints.append( uv.p1().toPoint() );
            }
        }
        manyPoints.append( list->last() ); // make sure to include the last point.
    }
    else
    {
        manyPoints = *list;
    }

    return manyPoints;
}

/*
 * setCalibrationScale
 *
 * Pass in the PxPerMm conversion scale. This is called by liveScene, and the conversion
 * scale comes from the value stored with the currently reviewed capture.
 * CaptureWidget -> Frontend -> LiveScene -> AreaMeasurementOverlay
 *
 * The Measurement App passes the diameter of the catheter in mm, and it is assumed
 * that a line the length of the catheter center is drawn. These values are calibrated
 * to determine the pxPerMm conversion.
 */
void AreaMeasurementOverlay::setCalibrationScale( const int CalValMm )
{
#ifdef MEASUREMENT_APP
    if( polygonPoints.size() >= 2 )
    {
        QLineF l( polygonPoints.at( 0 ), polygonPoints.at( 1 ) );

        currPxPerMm = (float)l.length() / ( (float)CalValMm / 1000 );

        qDebug() << "currPxPerMm: " << currPxPerMm << "(float)l.length()"
                 << (float)l.length() << "(float)CalValMm" << (float)CalValMm
                 << "currPxPerMm = (float)l.length() / (float)diameterMm / 1000" << ( (float)l.length() / (float)CalValMm / 1000 );
    }
#else
    currPxPerMm = CalValMm;
#endif
}

/*
 * addControlPoint
 *
 * If p is on line ( w/in tolerance between points in polygonPoints )
 * then add a new point in polygonPoints at p.
 *
 * Return true if a new point is added.
 */
bool AreaMeasurementOverlay::addControlPoint( QPoint p )
{
    if( polygonPoints.size() >= 2 )
    {
        // iterate through points
        for( int i = 0; i < polygonPoints.size(); i++ )
        {
            // draw rect around p, if rect line intersects rect, add control point at p.
            QRect r1( p.x() - PointTolerance, p.y() - PointTolerance, PointTolerance*2, PointTolerance*2 );
            QPolygon tmpP;
            if( i == ( polygonPoints.size() - 1 ) )
            {
                // This segment is from the last point to the first point (point that closes the polygon).
                tmpP << polygonPoints.at( i );
                tmpP << polygonPoints.at( 0 );
            }
            else
            {
                tmpP << polygonPoints.at( i );
                tmpP << polygonPoints.at( i + 1 );
            }
            QPolygon testPts = polygonToPoints( &tmpP );

            for( int j = 0; j < testPts.size(); j++ )
            {
                if( r1.contains( testPts.at( j ) ) )
                {
                    // overlap, so add control point
                    polygonPoints.insert( i + 1, p );
                    return true;
                }
            }
        }
    }

    return false;
}

/***********************************
 * Calculation functions ***********
 ***********************************/

/*
 * computeArea
 *
 * Compute the area enclosed by the polygon.
 *
 * Reference: Weisstein, Eric W. "Polygon Area." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/PolygonArea.html
 */
int AreaMeasurementOverlay::computeArea( void )
{
    int numPoints = polygonPoints.size();
    int tmp_px2 = 0;

    int j = numPoints - 1;

    for( int i = 0; i < numPoints; i++ )
    {
        tmp_px2 += ( polygonPoints.point( j ).x() * polygonPoints.point( i ).y() -
                     polygonPoints.point( i ).x() * polygonPoints.point( j ).y() );
        j = i;
    }

    return( abs( tmp_px2 / 2 ) );
}

/*
 * computeLength
 */
int AreaMeasurementOverlay::computeLength( QPoint *p1, QPoint *p2 )
{
    int x1 = p1->x();
    int y1 = p1->y();
    int x2 = p2->x();
    int y2 = p2->y();

    return int( qSqrt( ( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) ) );
}

/*
 * calculateLongestLine
 *
 * Finds the longest line traveling through the centroid. If MaxIterations
 * are required to find the max, rule that the max was not determined. Max Line is set
 * to 0 and returns FALSE. Returns TRUE on success.
 */
bool AreaMeasurementOverlay::calculateLongestLine( QPolygon *list )
{
    if( centroid.isValid )
    {
        if( list->containsPoint( centroid.point, Qt::OddEvenFill ) )
        {
            QList<int> lengths;

            int max = 0;
            for( int i = 0; i < list->size(); i++ )
            {
                QPoint p1 = list->point( i );
                lengths << computeLength( &p1, &centroid.point );
                if( lengths.at( i ) > max )
                {
                    max = lengths.at( i );
                    classMaxLine.setPoints( list->at( i ), centroid.point );
                }
            }

            // find longest line from max, through center, to side of polygon.
            if( !classMaxLine.isNull() )
            {
                const int MaxIterations = 1000;
                int j = 0;
                while( list->containsPoint( classMaxLine.toLine().p2(), Qt::OddEvenFill ) )
                {
                    classMaxLine.setLength( classMaxLine.length() + 20 );
                    j++;
                    if( j > MaxIterations )
                    {
                        classMaxLine = QLineF(); // construct a null line
                        return false;
                    }
                }

                // Possible overshoot at this point, work back the other direction until we hit the line.
                while( !list->containsPoint( classMaxLine.toLine().p2(), Qt::OddEvenFill ) )
                {
                    classMaxLine.setLength( classMaxLine.length() - 1 );
                }
            }
        }
    }

    return true;
}

/*
 * calculateShortestLine
 *
 * Finds the shortest line traveling through the centroid. If MaxIterations
 * are required to find the min, rule that the min was not determined. Min Line is set
 * to 0 and returns FALSE. Returns TRUE on success.
 */
bool AreaMeasurementOverlay::calculateShortestLine( QPolygon *list )
{
    if( centroid.isValid )
    {
        if( list->containsPoint( centroid.point, Qt::OddEvenFill ) )
        {
            QLineF minLine( 0,0,this->boundingRect().width(),this->boundingRect().height() );
            QLineF centerLine;
            // iterate through the list of vertices
            for( int i = 0; i < list->size(); i++ )
            {
                const int MaxIterations = 1000;
                int j = 0;
                centerLine.setPoints( list->at( i ), centroid.point );
                while( list->containsPoint( centerLine.toLine().p2(), Qt::OddEvenFill ) )
                {
                    centerLine.setLength( centerLine.length() + 20 );
                    j++;
                    if( j > MaxIterations )
                    {
                        classMinLine = QLineF(); // construct a null line
                        return false;
                    }
                }

                // Possible overshoot at this point, work back the other direction until we hit the line.
                while( !list->containsPoint( centerLine.toLine().p2(), Qt::OddEvenFill ) )
                {
                    centerLine.setLength( centerLine.length() - 1 );
                }

                if( !centerLine.isNull() )
                {
                    if( minLine.length() > centerLine.length() )
                    {
                        minLine = centerLine;
                    }
                }
            }
            if( !minLine.isNull() )
            {
                classMinLine = minLine;
            }
        }
    }

    return true;
}

/*
 * calculateCentroid
 *
 * Centroid is defined as the average of all vertex positions. If the centroid is not contained
 * by the polygon, Centroid_T::isValid is set to false.
 */
void AreaMeasurementOverlay::calculateCentroid( QPolygon *list )
{
    if( polygonPoints.size() > 2 )
    {
        QPoint c( 0, 0 );
        for( int i = 0; i < list->size(); i++ )
        {
            c += list->at( i );
        }
        c /= list->size();

        if( list->containsPoint( c, Qt::OddEvenFill ) )
        {
            centroid.isValid = true;
            centroid.point = c;
        }
        else
        {
            centroid.isValid = false;
            centroid.point = QPoint( 0, 0 );
        }
    }
}

/*
 * calculate
 *
 * Only needs to be updated when new points are added or changed.
 */
void AreaMeasurementOverlay::calculate()
{
    virtualPoints = polygonToPoints( &polygonPoints );
    calculateCentroid( &virtualPoints );
    calculateLongestLine( &virtualPoints );
    calculateShortestLine( &virtualPoints );
    polygonArea = computeArea();
}

/***********************************
 * Painting functions **************
 ***********************************/
/*
 * paintCalculationBox
 */
void AreaMeasurementOverlay::paintCalculationBox( QPainter *painter )
{
#ifndef MEASUREMENT_APP
    const int FontSize = 24;
#else
    const int FontSize = 14;
#endif
    QFont font = painter->font();
    font.setPointSize( FontSize );
    painter->setFont( font );
    const int xMargin = FontSize / 2;
    const int yMargin = FontSize / 2;

    if( polygonPoints.size() == 2 )
    {
#ifdef MEASUREMENT_APP
        if( currPxPerMm > 0 )
        {
            emit twoPointsDrawn();
        }
#endif
        if( currPxPerMm > 0 )
        {
            QLineF line;
            line.setPoints( polygonPoints.point( 0 ), polygonPoints.point( 1 ) );
            painter->setPen( QPen( QBrush( QColor( 255, 100, 0 ), Qt::SolidPattern ), 2 ) );
            QString str = QString( "Length: %1 mm" ).arg( QString::number( line.length() / double(currPxPerMm), 'f', MeasurementPrecision ) );
            painter->drawText( box->left() + xMargin, box->top() + font.pointSize() + yMargin, str );

            // Size the box according to the text drawn.
            QStaticText st( str );
            st.prepare( QTransform(), font );  // prepare text so we can determine the text size
            box->setWidth( int(st.size().width() + ( 2 * xMargin ) ) );
            box->setHeight( int(st.size().height() + ( 2 * yMargin ) ) );
#ifdef MEASUREMENT_APP
            painter->fillRect( *box, QColor( 255, 255, 255, 60 ) );
#endif
            painter->drawRect( *box );
        }
    }
    else
    {
        if( currPxPerMm > 0 )
        {
            if( centroid.isValid )
            {
                QRect minRect( box->left() + xMargin, box->top(), box->width() - xMargin, font.pointSize() + yMargin );
                painter->setPen( QPen( QBrush( QColor( 255, 100, 0 ), Qt::SolidPattern ), 2 ) );
                QString str( QString( "Min: %1 mm" ).arg( QString::number( classMinLine.length() / double(currPxPerMm), 'f', MeasurementPrecision ) ) );
                painter->drawText( minRect.bottomLeft(), str );

                QRect maxRect( box->left() + xMargin, minRect.bottom() + yMargin, box->width() - xMargin, font.pointSize() + yMargin );
                painter->setPen( QPen( QBrush( QColor( 255, 215, 0 ), Qt::SolidPattern ), 2 ) );
                str = QString( "Max: %1 mm" ).arg( QString::number( classMaxLine.length() / double(currPxPerMm), 'f', MeasurementPrecision ) );
                painter->drawText( maxRect.bottomLeft(), str );

                QRect areaRect( box->left() + xMargin, maxRect.bottom() + yMargin, box->width() - xMargin, font.pointSize() + yMargin );
                painter->setPen( QPen( QBrush( Qt::magenta, Qt::SolidPattern ), 2 ) );
                str = QString( "Area: %1 mm" ).arg( QString::number( polygonArea / double(currPxPerMm) / double(currPxPerMm), 'f', MeasurementPrecision ) );
                painter->drawText( areaRect.bottomLeft(), str );

                /*
                 * Draw a superscript "2" using drawText by finding the right edge of the drawn text and drawing a smaller font
                 * "2" in a higher position. This is not the preffered method, but we are having issues with drawStaticText().
                 */
                QStaticText staticText( str );
                staticText.prepare( QTransform(), font );
                QFont subFont = painter->font();
                subFont.setPointSize( FontSize - ( FontSize / 3 ) );
                painter->setFont( subFont );
                painter->drawText( QPoint( int(areaRect.left() + staticText.size().width()), areaRect.bottom() - ( FontSize / 2 ) ), QString( "2" ) );
                painter->setFont( font );

                // Size the box according to the text drawn.
                staticText.setText( QString( "Area: %1 mm^2" ).arg( QString::number( polygonArea / double(currPxPerMm) / double(currPxPerMm), 'f', MeasurementPrecision ) ) );
                staticText.prepare( QTransform(), font );                               // prepare text so we can determine the text size
                box->setWidth( int(staticText.size().width() ) );                             // set box width based on text
                box->setHeight( int( ( staticText.size().height() * 3 ) + ( 3 * yMargin ) ) ); // set box heigh based on text
#ifdef MEASUREMENT_APP
                box->setBottom( areaRect.bottom() + yMargin );
                painter->fillRect( *box, QColor( 255, 255, 255, 50 ) );
#endif
                painter->drawRect( *box );
            }
        }
    }
}

/*
 * paintPolygon
 */
void AreaMeasurementOverlay::paintPolygon( QPainter *painter )
{
    painter->setPen( QPen( QBrush( currentColor, Qt::SolidPattern ), 2 ) );

    // Make sure two points exist or the index goes out of range.
    if( polygonPoints.size() == 2 )
    {
        painter->drawLine( polygonPoints.at( 0 ), polygonPoints.at( 1 ) );
    }
    else if( polygonPoints.size() > 2 )
    {
        painter->drawPolygon( polygonPoints );
    }
}

/*
 * paintControlPoints
 */
void AreaMeasurementOverlay::paintControlPoints( QPainter *painter )
{
    for( int i = 0; i < polygonPoints.size(); i++ )
    {
        if( i == 0 ) // fill the first dot white
        {
            QBrush tmp = painter->brush();
            painter->setBrush( Qt::white );
            painter->setPen( QPen( QBrush( Qt::cyan, Qt::SolidPattern ), 2 ) );
            painter->drawEllipse( polygonPoints.at( i ), ( PointTolerance / 4 ), ( PointTolerance / 4 ) );
            painter->setBrush( tmp );
        }

        painter->setPen( QPen( QBrush( Qt::cyan, Qt::SolidPattern ), 2 ) );
        painter->drawEllipse( polygonPoints.at( i ), ( PointTolerance / 4 ), ( PointTolerance / 4 ) );
    }
}

/*
 * paintMinMax
 */
void AreaMeasurementOverlay::paintMinMax( QPainter *painter )
{
    if( enableDrawMinMax && centroid.isValid )
    {
        if( !classMaxLine.isNull() )
        {
            painter->setPen( QPen( QBrush( QColor( 255, 215, 0 ), Qt::SolidPattern ), 2 ) );
            painter->drawLine( classMaxLine );
        }
        if( !classMinLine.isNull() )
        {
            painter->setPen( QPen( QBrush( QColor( 255, 100, 0 ), Qt::SolidPattern ), 2 ) );
            painter->drawLine( classMinLine );
        }
    }
}
