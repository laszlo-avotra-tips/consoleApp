/*
 * gridOverlay.h
 *
 * Overlay a grid on an image.  The grid
 * spacing is configurable.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc
 */

#ifndef GRIDOVERLAY_H
#define GRIDOVERLAY_H

#include <QLabel>


class gridOverlay : public QLabel
{
    Q_OBJECT

public:
    gridOverlay( QWidget *parent = 0 );
    void setSpacing( int val )
    {
        numRadialLines = val;
        angle = 360.0 / (float)val;
    }

private:
    int numRadialLines;
    float angle;
    void paintEvent( QPaintEvent *event );

protected:
};

#endif // GRIDOVERLAY_H
