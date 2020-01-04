/*
 * gridOverlay.cpp
 * 
 * Overlay a polar grid on an image.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc
 */

#include "gridOverlay.h"
#include <QPainter>


const int DefaultNumberRadialLines = 12;

/*
 * Constructor
 */
gridOverlay::gridOverlay( QWidget *parent ) : QLabel( parent )
{
    numRadialLines = DefaultNumberRadialLines;
    angle = 360.0f / DefaultNumberRadialLines;
}


/*
 * paintEvent()
 *
 * Redraw when needed
 */
void gridOverlay::paintEvent( QPaintEvent * )
{
    const int X1 = this->height() / 2;
    const int Y1 = this->width()  / 2;
    const QPoint ImageCenter( X1, Y1 );

    const int InnerRadius_px = 50;
    const int spacing_px     = 50;

    QPainter p;

    p.begin( this );

    p.setPen( QColor( Qt::yellow ).darker( 300 ) );

    /*
     * draw overlay rings
     */
    const int NumReticles = 5;
    int radius            = InnerRadius_px;

    for( int i = 0; i < NumReticles; i++ )
    {
        p.drawEllipse( ImageCenter, radius, radius );
        radius += spacing_px;
    }

    /*
     * draw radial lines
     */
    p.translate( ImageCenter );
    for( int kr = 0; kr < numRadialLines; kr++ )
    {
        p.drawLine( InnerRadius_px, 0, radius - spacing_px, 0 );
        p.rotate( double(angle) );
    }

    p.end();
}
