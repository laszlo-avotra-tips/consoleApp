/*
 * annotateoverlay.cpp
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 */
#include <QPainter>
#include <QDebug>
#include "defaults.h"
#include "annotateoverlay.h"

/*
 * Constructor
 */
AnnotateOverlay::AnnotateOverlay( QGraphicsItem *parent )
    : QGraphicsPixmapItem( parent )
{
    // Set the drawing surface
    overlayPixmap = new QPixmap( SectorWidth_px, SectorHeight_px + WaterfallHeight_px );
    setPixmap( *overlayPixmap );

    currentColor    = Qt::blue;
    currentPenWidth = 20;
    mouseIsDown     = false;

    workingSegmentList = nullptr;
}

/*
 * Destructor
 *
 * Free all memory created.
 */
AnnotateOverlay::~AnnotateOverlay()
{
    /*
     * Do not explicitly delete workingSegmentList; it is always the last item
     * in drawingsList once mouseReleaseEvent() has been called.
     */

    if( overlayPixmap )
    {
        delete overlayPixmap;
    }

    if( !drawingsList.isEmpty() )
    {
        qDeleteAll( drawingsList );
    }
}

/*
 * mousePressEvent
 *
 * Start a new drawing
 */
void AnnotateOverlay::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    mouseIsDown = true;

    workingSegmentList = new QList<QPoint>;

    // Save the starting point to the list
    workingSegmentList->append( event->pos().toPoint() );
}

/*
 * mouseMoveEvent
 *
 * Update the drawing's list of points
 */
void AnnotateOverlay::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    if( mouseIsDown )
    {
        // Add each point as the mouse moves
        workingSegmentList->append( event->pos().toPoint() );
    }
}

/*
 * mouseReleaseEvent
 *
 * Finish this drawing and add it to the list of all drawings. QList takes
 * ownership of the pointers.
 */
void AnnotateOverlay::mouseReleaseEvent( QGraphicsSceneMouseEvent * /*event*/ )
{
    mouseIsDown = false;
    drawingsList.append( workingSegmentList );
}

/*
 * paint
 *
 * Draw all annotations on the overlay and display it
 */
void AnnotateOverlay::paint( QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/ )
{
    painter->setPen( QPen( QBrush( currentColor, Qt::SolidPattern ), currentPenWidth ) );
    painter->setOpacity( 0.3 ); // 30 percent transparent

    /*
     * Draw the figure that is currently being created; don't redraw it when the
     * mouse is released.
     */
    if( mouseIsDown )
    {
        paintSegments( painter, workingSegmentList );
    }

    /*
     * Draw all of the figures that have been created during this annotation session
     */
    if( !drawingsList.isEmpty() )
    {
        // Go through each drawing one-by-one
        QList<QPoint> *segList;
        foreach( segList, drawingsList )
        {
            paintSegments( painter, segList );
        }
    }
//#if DEBUG_ANNOTATION
//    painter->drawRect( 0, 0, overlayPixmap->width(), overlayPixmap->height() );
//#endif
}

/*
 * paintSegments
 *
 * Paint a polyline from a list of segments.
 */
void AnnotateOverlay::paintSegments( QPainter *painter, QList<QPoint> *segmentList )
{
    // Only draw if a list exists and there are at least 2 points in it
    if( segmentList && ( segmentList->size() > 2 ) )
    {
        QVector<QPoint> vect = segmentList->toVector();
        painter->drawPolyline( vect );
    }
}
