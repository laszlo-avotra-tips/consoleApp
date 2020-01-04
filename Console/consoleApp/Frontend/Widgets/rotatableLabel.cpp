/*
 * rotatableLabel.cpp
 *
 * The rotatableLabel is, well, a label that can be rotated by grabbing
 * and dragging. Used in the lag correction wizard.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger
 */


#include "rotatableLabel.h"
#include <QPainter>
#include <QMatrix>
#include <math.h>

//const double pi(3.1415);

rotatableLabel::rotatableLabel( QWidget *parent ) : QLabel( parent )
{
    currentAngle = 0;
}


/*
 * paintEvent()
 *
 * Redraw at the requested angle
 */
void rotatableLabel::paintEvent( QPaintEvent * )
{
    QPainter p;
    QMatrix m;
    QPointF center = this->rect().center();
    double yTranslation = center.y() + 0.5;
    double xTranslation = center.x() + 0.5;

    m.translate(xTranslation, yTranslation);
    m.rotate(currentAngle);
    m.translate(-xTranslation, -yTranslation);
    p.begin(this);
    p.setWorldMatrix(m);
    p.drawPixmap( 0, 0, this->width(), this->height(), *(pixmap()) );
    p.end();
}
