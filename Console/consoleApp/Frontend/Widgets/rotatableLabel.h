/*
 * rotatableLabel.h
 *
 * The rotatableLabel is, well, a label that can be rotated by grabbing
 * and dragging. Used in the lag correction wizard.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger
 */

#ifndef ROTATEABLELABEL_H
#define ROTATEABLELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPointF>

class rotatableLabel : public QLabel
{

    Q_OBJECT

public:
    rotatableLabel( QWidget *parent = 0 );
    double getAngle( void ) {
        return( currentAngle );
    }
    void setAngle( int angle ) {
        currentAngle = angle;
        update();
    }

private:
    double currentAngle;
    void paintEvent( QPaintEvent *event );

protected:
};

#endif // ROTATEABLELABEL_H
