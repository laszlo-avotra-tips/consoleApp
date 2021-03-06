/*
 * waterfall.cpp
 *
 * Implements the scrolling time vs. depth display widget for OCT images.
 * The waterfall is displayed on the
 * screen as a scrolling graphic with a width greater than
 * its height (i.e., 1024 pixels wide by 512 high).
 * Internally it is represented by a graphic that is 512 px wide
 * by 1024 px high. The widget is rotated 90 degrees when drawn
 * to the screen. This is done to support portrait oriented
 * physician displays.
 *
 * Author: Dennis W. Jackson and Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#include <QPainter>
#include <QImage>
#include "waterfall.h"
#include "defaults.h"
#include <math.h>
#include <string.h>   // memmove_s
#include "deviceSettings.h"
#include "profiler.h"
#include <QDebug>
#include "Utility/userSettings.h"

/*
 * constructor
 */
waterfall::waterfall( int w, int h, QGraphicsItem *parent )
    :  QGraphicsPixmapItem(parent), width(w), height(h)
{
    status = 1;

    // Set up our drawing surface
    wfImage = new QImage( width, height, QImage::Format_Indexed8 );
    if ( !wfImage ) {
        status = 0;
        return;
    }

    wfImage->fill( 0x00 ); // Black background
    skipCounter = 0;
    glueLineOffset_px = 0;
    clearCache = false;
}

waterfall::~waterfall()
{
    if ( wfImage ) {
       delete wfImage;
    }
}

/*
 * render()
 *
 * Update the waterfall image.
 */
void waterfall::render(void)
{
    scrollWaterfall();

    setPixmap( QPixmap::fromImage( *wfImage ) );
}


/*
 * scrollWaterfall
 *
 * Add the latest lines in the cache to the image and scroll.
 */
void waterfall::scrollWaterfall(void)
{
    TIME_THIS_SCOPE( wf_scrollWaterfall );

    // grab the lock before moving data
    dataAccessMutex.lock();

    QByteArray lineData;
    int count = lineCache.count();

    // If the cache ended up with more lines than the size of the waterfall (possible
    // if drawing gets delayed at the max waterfall scroll rate), limit the amount of
    // data we move and clear out the rest (see below)
    if( count > wfImage->height() )
    {
        count = wfImage->height() - 1;
        clearCache = true;
    }

    // Amount of data in a row of the image
    int dataSize = int(sizeof(char)) * wfImage->width();

    // Get direct access to the image data
    unsigned char *rawPixels = static_cast<unsigned char *>(wfImage->bits() );

    // Shift all data linecount rows down
    unsigned char *dstaddr = rawPixels + ( dataSize * count );
    const size_t length = size_t(dataSize * ( wfImage->height() - count ) );
    memmove_s( dstaddr, length, rawPixels, length );

    // Consume the lines in the cache to update the waterfall.  Use the count
    // that was determined above instead of lineCache.count() since lines
    // may have been added since the count was assigned.  
    while( count > 0 )
    {
        // Pull the line into local data
        lineCacheLock.lock();
        lineData = lineCache.takeFirst();
        lineCacheLock.unlock();

        // copy the line data into the waterfall.
        memcpy( rawPixels + (count * dataSize),
                lineData,
                size_t(dataSize ) );

        // Clear up to the glue line offset using the same color as the catheter.
        memset( rawPixels + (count * dataSize),
                0, // Keep the color black
                size_t(glueLineOffset_px ) );
        // Remove the local line data
        count--;
    }

    // the cache was bigger than the size of the waterfall; clear out
    // what wasn't used
    if( clearCache )
    {
        lineCacheLock.lock();
        lineCache.clear();
        lineCacheLock.unlock();
        clearCache = false;
    }

    dataAccessMutex.unlock();
}

/*
 * addLine
 *
 * Updates the waterfall image by shifting all current data
 * to the left and adding the new line of data to the
 * right-most column.
 */
void waterfall::addLine( QSharedPointer<scanframe> &data )
{
    // copy the a-line data locally
    QByteArray lineData = data->dispData->mid( 0, MaxALineLength / 2 );  // XXX: needs update for deep view variable names

    const int DropModulus = 33; // Historically from 33ms update rate.

    userSettings &settings = userSettings::Instance();

    // Drop some interval so that we don't speed along too fast
    // when given lots of data: making the data buffer bigger
    //  would work too, but then everything slows down as we
    // scroll.
    skipCounter = ( ( skipCounter + 1 ) % DropModulus );
    if( skipCounter == ( DropModulus - settings.waterfallRate() ) )
    {
        skipCounter = 0;

        // Add the line to the cache for later drawing
        lineCacheLock.lock();
        lineCache.append( lineData );
        lineCacheLock.unlock();
    }
}

/*
 * clear
 *
 * Clear the image
 *
 */
void waterfall::clearImage( void )
{ 
    // Clear the image
    wfImage->fill( 0x00 );
}

/*
 * freeze
 *
 * Produce a QImage of the current waterfall, suitable for capturing
 * and return to the caller.
 */
QImage waterfall::freeze( void )
{
    // grab the lock before copying
    dataAccessMutex.lock();
    QImage snapshot = wfImage->copy();
    dataAccessMutex.unlock();

    return snapshot;
}

/*
 * deviceChanged
 *
 * The user has changed the device being used. Update the device specific
 * values cached here to reflect that.
 */
void waterfall::deviceChanged(void)
{
    deviceSettings &dev = deviceSettings::Instance();
    glueLineOffset_px = dev.current()->getInternalImagingMask_px();
}

/*
 * updateColorMap
 *
 * Change the color map used to display the 8-bit image
 */
void waterfall::updateColorMap( QVector<QRgb> map )
{
    wfImage->setColorTable( map );
}
