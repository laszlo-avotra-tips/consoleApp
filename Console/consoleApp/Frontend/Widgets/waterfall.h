/*
 * waterfall.h
 *
 * Waterfall display widget. Displays a scrolling time vs.
 * depth display of OCT data. The waterfall is displayed on the
 * screen as a scrolling graphic with a width greater than
 * its height (i.e., 1024 pixels wide by 512 high).
 * Internally it is represented by a graphic that is 512 px wide
 * by 1024 px high. The widget is rotated 90 degrees with drawn
 * to the screen. This is done to support portrait oriented
 * physician displays.
 *
 * Author: Dennis W. Jackson and Chris White
 *
 * Copyright (c) 2009-2018 Avinger Inc.
 */

#pragma once

#include <QMap>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QSharedPointer>
#include <QMutex>
#include <QMatrix>
#include "scanLine.h"
#include "defaults.h"
#include "buildflags.h"

class waterfall : public QGraphicsPixmapItem
{
public:
    waterfall( int width = WaterfallHeight_px, int height = WaterfallWidth_px, QGraphicsItem *parent = 0);
    ~waterfall();
    QImage freeze( void );

    void addLine( QSharedPointer<scanframe> &data );
    void render( void );
    void deviceChanged( void );
    void setSkipCount( int skipEvery );
    void clearImage( void );
    int  getStatus( void ) { return status; }
    int  getHeight( void ) { return height; }
    int  getWidth(  void ) { return width; }

    void sample( char *destBuffer ) {
        QMatrix m;
        m.rotate(90.0);
        QImage transformed = wfImage->transformed(m);
        int w = transformed.width();
        int h = transformed.height();
        memcpy( destBuffer, transformed.bits(), w * h );
    }

    void updateColorMap(QVector<QRgb> map );

public slots:

private:
    int status;

    QImage *wfImage;
    int  width;
    int  height;
    int  skipCounter;
    int  glueLineOffset_px;
    bool clearCache;   

    // Protect the line cache from simultaneous access
    QMutex lineCacheLock;
    QList< QByteArray > lineCache;

    // Protect the raw data from simultaneous access when scrolling and freezing
    QMutex dataAccessMutex;

    void scrollWaterfall(void);
};
