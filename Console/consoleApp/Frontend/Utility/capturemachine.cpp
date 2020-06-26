/*
 * captureMachine.cpp
 *
 * The capture machine object handles all file and database
 * operations for image captures. It provides the model
 * and abstraction layer for dealing with the current
 * set of captures.
 *
 * No patient information is written to the capture images.
 *
 * Author: Chris White
 * Copyright (c) 2010-2018 Avinger, Inc.
 */

#include "capturemachine.h"
#include "captureListModel.h"
#include "defaults.h"
#include "userSettings.h"
#include <QDateTime>
#include <QDir>
#include <QPainter>
#include "logger.h"

/*
 * constructor
 */
captureMachine::captureMachine()
{
    currCaptureNumber = 0;

    // Connect model signals
    captureListModel &capList = captureListModel::Instance(); // Should have valid caseinfo
    connect( &capList, SIGNAL( warning( QString ) ), this, SIGNAL( warning( QString ) ) );
}

// TBD: merge capture and clipCapture.  eliminate duplicate code!  See #470.


/*
 * capture()
 *
 * Given the sector image (and procedure data TBD),
 * add the images to the file archive and the database.
 */
void captureMachine::imageCapture( QImage decoratedImage, QImage sector, QString tagText, unsigned int timestamp, int pixelsPerMm, float zoomFactor )
{
    // create the item to put on the queue for processing
    CaptureItem_t c;

    c.decoratedImage = decoratedImage;
    c.sectorImage    = sector;
    c.tagText        = tagText;
    c.timestamp      = timestamp;
    c.pixelsPerMm    = pixelsPerMm;
    c.zoomFactor     = zoomFactor;

    mutex.lock();
    captureQ.enqueue( c );
    runThreadQ.enqueue( true );
    mutex.unlock();

    if( !isRunning() )
    {
        // kick-off the thread
        start();
    }
}

/*
 * processCapture
 *
 * Paint session data on the image and store it to disk.
 */
void captureMachine::processImageCapture( CaptureItem_t captureItem )
{
    // TBD: cannot be global to the class?
    const QImage LogoImage( ":/octConsole/Frontend/Resources/logo-top.png" );

    QImage sectorImage( captureItem.sectorImage.convertToFormat( QImage::Format_RGB32 ) ); // Can't paint on 8-bit

    deviceSettings &devSettings = deviceSettings::Instance();

    // Obtain the current timestamp
    const QDateTime currTime = QDateTime().fromTime_t( captureItem.timestamp );

    // capture number is tracked here
    currCaptureNumber++;
    QString strCaptureNumber = QString( "%1" ).arg( currCaptureNumber, 3, 10, QLatin1Char( '0' ) );

    //
    caseInfo &info = caseInfo::Instance();
    QString saveDirName = info.getCapturesDir();
    QString saveName =  QString( ImagePrefix ) + strCaptureNumber;

    const int logoX{ SectorWidth_px - LogoImage.width() - 100};
    const int logoY{50};

    /*
     * Paint the procedure data to the sector image.
     */
    QPainter painter( &sectorImage );

    addTimeStamp(painter);
    addFileName(painter,saveName);

    //    Upper Right -- Logo
    painter.drawImage( logoX, logoY, LogoImage );

    painter.end();


    // Store the capture
    const QString SecName            = saveDirName + "/"        + saveName + SectorImageSuffix    + ".png";
    const QString ThumbSecName       = saveDirName + "/.thumb_" + saveName + SectorImageSuffix    + ".png";
    const QString DecoratedImageName = saveDirName + "/"        + saveName + DecoratedImageSuffix + ".png";

    QMatrix m;
//    m.rotate( 90 );
//    qDebug() << __FUNCTION__ << ":" << __LINE__ <<" sector sectorImage.width()=" << sectorImage.width() << ", sectorImage.height()=" << sectorImage.height();
    LOG3(SecName,saveDirName,saveName)
    auto imageRect = sectorImage.rect();
    if( !sectorImage.save( SecName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Image Capture: sector capture failed" )
    }
    else
    {
        emit sendFileToKey( SecName );
    }
    qDebug() << __FUNCTION__ << ":" << __LINE__ << " imageRect.x()=" << imageRect.x() << " imageRect.y()=" <<imageRect.y();

    // save a thumbnail image for the UI to use
    if( !sectorImage.scaled( ThumbnailHeight_px, ThumbnailWidth_px ).save( ThumbSecName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Image Capture: sector thumbnail capture failed" )
    }
    else
    {
        emit sendFileToKey( ThumbSecName );
    }

    /*
     * save the decorated image
     */
    // rotate the decorated image to match the display
//    m.rotate( 90 );

    // Paint the logo on the decorated image in the upper right corner
    QImage decoratedImage( captureItem.decoratedImage.convertToFormat( QImage::Format_RGB32 ) ); // Can't paint on 8-bit
    painter.begin( &decoratedImage );

    addTimeStamp(painter);
    addFileName(painter,saveName);

    painter.drawImage( logoX, logoY, LogoImage );
    painter.end();

    QImage dim = decoratedImage.copy(imageRect);

    if( !dim.save( DecoratedImageName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Image Capture: decorated image capture failed" )
    }
    else
    {
        emit sendFileToKey( DecoratedImageName );
    }
    qDebug() << __FUNCTION__ << ": decoratedImage.width()=" << dim.width() << ", decoratedImage.height()" << dim.height();

    // update the model
    captureListModel &capList = captureListModel::Instance(); // Should have valid caseinfo
    if( capList.addCapture( captureItem.tagText,
                            currTime.toTime_t(),
                            saveName,
                            devSettings.current()->getDeviceName(),
                            //devSettings.current()->isHighSpeed(),
                            true,
                            captureItem.pixelsPerMm,
                            captureItem.zoomFactor ) < 0 )
    {
        return;   // Failure warnings generated in the call
    }

    emit updateCaptureCount();

    emit sendCaptureTag( saveName );
    LOG( INFO, QString( "Capture - %1" ).arg( saveName ) )
}


/*
 * clipCapture()
 *
 * Given the sector image (and procedure data TBD),
 * add the images to the file archive and the database.
 */
void captureMachine::clipCapture( QImage sector, QString strClipNumber, unsigned int timestamp )
{
    // create the item to put on the queue for processing
    ClipItem_t c;

    c.sectorImage    = sector;
    c.strClipNumber  = strClipNumber;
    c.timestamp      = timestamp;

    mutex.lock();
    clipQ.enqueue( c );
    runThreadQ.enqueue( true );
    mutex.unlock();

    if( !isRunning() )
    {
        // kick-off the thread
        start();
    }
}

/*
 * processClip
 *
 * Paint session data on the image and store it to disk.
 */
void captureMachine::processLoopRecording( ClipItem_t loop )
{
    const QImage LogoImage( ":/octConsole/Frontend/Resources/logo-top.png" );

    caseInfo &info = caseInfo::Instance();
    const QString ClipName = "clip-" + loop.strClipNumber;

    QImage secRGB( loop.sectorImage.convertToFormat( QImage::Format_RGB32 ) ); // Can't paint on 8-bit

    // Obtain the current timestamp
    const QDateTime currTime = QDateTime().fromTime_t( loop.timestamp );

    /*
     * Paint information on the sector image that needs to be visible when reviewed during a case
     */
    QPainter painter( &secRGB );

    //    Upper Right -- Logo
    painter.drawImage( SectorWidth_px - LogoImage.width() - 100, 50, LogoImage );

    painter.end();

    // Build the location directory
    QString saveDirName = info.getClipsDir();
    QString saveName =  QString( ClipName );

    // Store the capture
    const QString SecName      = saveDirName + "/"        + saveName + SectorImageSuffix    + ".png";
    const QString ThumbSecName = saveDirName + "/.thumb_" + saveName + SectorImageSuffix    + ".png";

    QMatrix m;
    m.rotate( 90 );


    if( !secRGB.save( SecName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Loop capture: sector capture failed" )
    }
    else
    {
        emit sendFileToKey( SecName );
    }

    // save a thumbnail image for the UI to use
    if( !secRGB.scaled( ThumbnailHeight_px, ThumbnailWidth_px ).save( ThumbSecName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Loop capture: sector thumbnail capture failed" )
    }
    else
    {
        emit sendFileToKey( ThumbSecName );
    }

    emit updateClipCount();

    emit sendCaptureTag( ClipName );
    LOG( INFO, "Loop Capture: " + ClipName )
}

void captureMachine::addTimeStamp(QPainter& painter)
{
    const int nowX{100};
    const int nowDateY{100};
    const int nowTimeY{160};

    painter.setPen( QPen( Qt::white ) );

    const auto& now = QDateTime::currentDateTime().toUTC();
    QString timeStampDate = now.toString("yyyy-MM-dd" );
    QString timeStampTime = now.toString("  hh:mm:ss");

    painter.setFont( QFont( "DinPro-regular", 20 ) );
    painter.drawText( nowX, nowDateY, timeStampDate);
    painter.drawText( nowX, nowTimeY, timeStampTime);
}

void captureMachine::addFileName(QPainter &painter, const QString &fn)
{
    const int fnX{100};
    const int fnY{2100};

    painter.setPen( QPen( Qt::white ) );

    painter.setFont( QFont( "DinPro-regular", 20 ) );
    painter.drawText( fnX, fnY, fn);
}


/*
 * run
 *
 * main loop for the thread
 */
void captureMachine::run( void )
{
    /*
     * Captures and clip captures can occur at almost any time.  The runThread
     * queue is really an event queue for all capture/clip events so we don't
     * have to have two threads (one for the clip queue and one for the capture queue).
     */
    while( !runThreadQ.empty() )
    {
        // process image captures
        while( !captureQ.empty() )
        {
            mutex.lock();
            CaptureItem_t f = captureQ.dequeue();
            mutex.unlock();

            processImageCapture( f );
        }

        // process clip captures
        while( !clipQ.empty() )
        {
            mutex.lock();
            ClipItem_t f = clipQ.dequeue();
            mutex.unlock();

            processLoopRecording( f );
        }

        // dequeue and toss the element
        mutex.lock();
        runThreadQ.dequeue();
        mutex.unlock();
    }
}
