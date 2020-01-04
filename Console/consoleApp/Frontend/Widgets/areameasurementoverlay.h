/*
 * areameasurementoverlay.h
 *
 * Author: Dennis W. Jackson, Ryan F. Radjabi
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 */
#ifndef OVERLAY_H
#define OVERLAY_H

#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

class AreaMeasurementOverlay : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    AreaMeasurementOverlay( QWidget *parent = nullptr );
    ~AreaMeasurementOverlay();

    void setColor( QColor color ) { currentColor = color; }
    void drawMinMax( bool state ) { enableDrawMinMax = state; }
    void setCalibrationScale( const int CalValMm );

signals:
#ifdef MEASUREMENT_APP
    void twoPointsDrawn();
#endif

private:
    QPixmap *overlayPixmap;

    QRect *box;
    int    boxDx;
    int    boxDy;
    int    boxCoordX;
    int    boxCoordY;
    bool   clickInBox;

    QPolygon polygonPoints;
    QPolygon virtualPoints;
    QColor   currentColor;
    bool     mouseIsDown;
    bool     allowReplace;
    bool     enableDrawMinMax;
    int      replacementPointIndex;
    float    currPxPerMm;
    int computeArea( void );
    int computeLength( QPoint *p1, QPoint *p2 );
    int pointsOverlap( QPoint *p1, QPolygon *list, int tolerance );
    bool calculateLongestLine( QPolygon *polygonPoints );
    bool calculateShortestLine( QPolygon *polygonPoints );
    QPolygon polygonToPoints( QPolygon *list );
    bool addControlPoint( QPoint p );
    void calculateCentroid( QPolygon *list );

    struct Centroid_T{
        QPoint point;
        bool isValid;
    };
    Centroid_T centroid;


    void paintCalculationBox( QPainter *painter );
    void paintPolygon( QPainter *painter );
    void paintControlPoints( QPainter *painter );
    void paintMinMax( QPainter *painter );
    void calculate();

    QLineF classMinLine;
    QLineF classMaxLine;
    int polygonArea;

protected:
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
};


#endif // OVERLAY_H
