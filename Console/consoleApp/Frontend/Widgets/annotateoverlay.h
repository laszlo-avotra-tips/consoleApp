/*
 * annotateoverlay.h
 *
 * Allow the Technician user to draw on the live image. Annotations
 * appear on both the Technician and Physician monitors.  Any number of
 * annotations can be added to the image.
 *
 * Each drawing is stored as a list of segments in drawingsList; this
 * QList is traversed in the paint() event for each drawing.
 *
 * This simple implementation only allows freehand drawing in one color and pen
 * width.  It can be enhanced as needed in the future.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 */
#ifndef ANNOTATEOVERLAY_H
#define ANNOTATEOVERLAY_H

#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

class AnnotateOverlay : public QGraphicsPixmapItem
{

public:
    AnnotateOverlay( QGraphicsItem *parent = nullptr );
    ~AnnotateOverlay();

    void setColor( QColor color ) { currentColor = color; }
    void setPenWidth( int width ) { currentPenWidth = width; }

private:
    QPixmap *overlayPixmap;
    QColor   currentColor;
    int      currentPenWidth;
    bool     mouseIsDown;

    QList<QPoint> *workingSegmentList;
    QList<QList<QPoint> *> drawingsList;

    void paintSegments( QPainter *painter , QList<QPoint> *segmentList );

protected:
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
};

#endif // ANNOTATEOVERLAY_H
