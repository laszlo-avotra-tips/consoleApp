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
#include <QFontMetrics>

#include "logger.h"
#include "Utility/octFrameRecorder.h"

/*
 * constructor
 */
captureMachine::captureMachine()
{
    currCaptureNumber = 0;

    // Connect model signals
    LOG1(currCaptureNumber)
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

    QString imageName = generateImageName();

    // Paint the decorated image
    QPainter painter;
    QImage decoratedImage( captureItem.decoratedImage.convertToFormat( QImage::Format_RGB32 ) ); // Can't paint on 8-bit
    painter.begin( &decoratedImage );

    addTimeStamp(painter);
    addFileName(painter,imageName);
    addCatheterName(painter);
    addLogo(painter);

    painter.end();

    // Store the capture
    saveImage(decoratedImage, imageName);
    saveThumbnail(decoratedImage, imageName);

    // add to the model
    addCaptureToTheModel(captureItem, imageName);

    LOG( INFO, QString( "Capture - %1" ).arg( imageName ) )
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

    LOG2(strClipNumber, timestamp)

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
void captureMachine::processLoopRecording(ClipItem_t clipItem )
{
    const QString clipFileName = generateClipFileName(clipItem);

    const QString clipName{clipItem.strClipNumber};
    auto* recorder = OctFrameRecorder::instance();
    recorder->setClipName(clipName);

    // Obtain the current timestamp
    const QDateTime currTime = QDateTime().fromTime_t( clipItem.timestamp );
    recorder->setTimeStamp(currTime.toString("hh:mm:ss"));
    /*
     * Paint information on the sector image that needs to be visible when reviewed during a case
     */
    QImage secRGB( clipItem.sectorImage.convertToFormat( QImage::Format_RGB32 ) ); // Can't paint on 8-bit
    QPainter painter( &secRGB );

    addLogo(painter, true);
    addTimeStamp(painter,true);
    addFileName(painter,clipName, true);
    addCatheterName(painter,true);

    painter.end();

    // Build the location directory
    caseInfo &info = caseInfo::Instance();
    QString saveDirName = info.getClipsDir();
    QString saveName =  QString( clipFileName );

    // Store the capture
    const QString clipThumbNailFileName = saveDirName + "/thumb_" + saveName + ".png";
    LOG1(clipThumbNailFileName)

    QImage clipThumbNail = secRGB.scaled( ThumbnailHeight_px, ThumbnailWidth_px );

    for (int ii = 0; ii < clipThumbNail.width(); ii++) {
        for (int jj = 0; jj < clipThumbNail.height(); jj++) {
            int gray = qGray(clipThumbNail.pixel(ii, jj));
            clipThumbNail.setPixel(ii, jj, QColor(gray, gray, gray).rgb());
        }
    }

    // save a thumbnail image for the UI to use
    if( !clipThumbNail.save( clipThumbNailFileName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Loop capture: sector thumbnail capture failed" )
    }
    else
    {
        emit sendFileToKey( clipThumbNailFileName );
    }

    LOG( INFO, "Loop Capture: " + clipFileName )
}

void captureMachine::addTimeStamp(QPainter& painter, bool isClip)
{
    int nowX{20};
    int firstRow{180};

    painter.setPen( QPen( Qt::white ) );

    const auto& now = QDateTime::currentDateTime().toUTC();
    QString timeStampTime = now.toString("hh:mm:ss");

    if(isClip){
        painter.setFont( QFont( "DinPro-regular", 8 ) );
        nowX = 10;
        firstRow = 90;
    } else {
        painter.setFont( QFont( "DinPro-regular", 20 ) );
    }
    painter.drawText( nowX, firstRow, timeStampTime);
}

void captureMachine::addFileName(QPainter &painter, const QString &fn, bool isClip)
{
    int fnX{20};
    int fnY{2140};

    painter.setPen( QPen( Qt::white ) );

    if(isClip){
        painter.setFont( QFont( "DinPro-regular", 8 ) );
        fnX = 10;
        fnY = 1000;
    } else {
        painter.setFont( QFont( "DinPro-regular", 20 ) );
    }
    painter.drawText( fnX, fnY, fn);
    LOG1(fn)
    LOG2(fnX,fnY)
}

void captureMachine::addCatheterName(QPainter &painter, bool isClip)
{
    QFont nameFont;

    if(isClip){
        nameFont = QFont("DinPro-regular", 8 );
    } else {
        nameFont = QFont("DinPro-regular", 20 );
    }

    painter.setFont(nameFont);

    QFontMetrics qfm(nameFont);

    LOG2(nameFont.pointSize(),nameFont.pointSizeF())
    LOG2(qfm.maxWidth(), qfm.height())

    auto device = deviceSettings::Instance().current();
    auto name = device->getSplitDeviceName();

    QStringList names = name.split("\n");
    QRect rect0 = qfm.tightBoundingRect(names[0]);
    QRect rect1 = qfm.tightBoundingRect(names[1]);

    LOG2(rect0.width(), rect0.height())
    LOG2(rect1.width(), rect1.height())

    int catheterX0{ int(SectorWidth_px * IMAGE_SCALE_FACTOR - rect0.width()) - 120 };
    int catheterX1{ int(SectorWidth_px * IMAGE_SCALE_FACTOR - rect1.width()) - 120 };
    int catheterY0{60};
    int catheterY1{120};

    LOG2(catheterX0, catheterX1)

    if(names.count() >= 2){
        if(isClip){
            catheterX0 = int(SectorWidth_px - rect0.width() ) - 110;
            catheterX1 = int(SectorWidth_px - rect1.width() ) - 110;
            catheterY0 = 30;
            catheterY1 = 60;
        }
        painter.drawText(catheterX0, catheterY0, names[0]);
        painter.drawText(catheterX1, catheterY1, names[1]);
    }
}

void captureMachine::addLogo(QPainter &painter, bool isClip)
{
    const int logoX0{20};
    const int logoY{20};
    const QImage logoImage( ":/octConsole/captureLogo.png" );
    int scale{360};

    if(isClip){
        scale = double(scale) / IMAGE_SCALE_FACTOR;
    }
    const QImage LogoImage = logoImage.scaledToWidth(scale);
    painter.drawImage( logoX0, logoY, LogoImage );
}

QString captureMachine::generateImageName()
{
    currCaptureNumber = captureListModel::Instance().countOfCapuredItems();
    currCaptureNumber++;
    QString strCaptureNumber = QString( "%1" ).arg( currCaptureNumber);

    return QString( ImagePrefix ) + strCaptureNumber;
}

void captureMachine::saveImage(const QImage&decoratedImage, const QString &imageName)
{
    caseInfo &info = caseInfo::Instance();
    QString saveDirName = info.getCapturesDir();
    const QString imageFileName = saveDirName + "/"        + imageName + ".png";
    if( !decoratedImage.save( imageFileName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Image Capture: decorated image capture failed" )
    }
    else
    {
        emit sendFileToKey( imageFileName );
    }
}

void captureMachine::saveThumbnail(const QImage &decoratedImage, const QString &imageName)
{

    caseInfo &info = caseInfo::Instance();
    // save a thumbnail image for the UI to use
    QImage thumbNail = decoratedImage.scaled( ThumbnailHeight_px, ThumbnailWidth_px );
    QString saveDirName = info.getCapturesDir();
    LOG2(thumbNail.width(), thumbNail.height())
    const QString thumbFileName = saveDirName + "/.thumb_" + imageName + ".png";

    if( !thumbNail.save( thumbFileName, "PNG", 100 ) )
    {
        LOG( DEBUG, "Image Capture: sector thumbnail capture failed" )
    }
    else
    {
        emit sendFileToKey( thumbFileName );
    }
}

void captureMachine::addCaptureToTheModel(const captureMachine::CaptureItem_t& captureItem, const QString &imageName)
{
    const QDateTime currTime = QDateTime::currentDateTime();
    captureListModel &capList = captureListModel::Instance(); // Should have valid caseinfo
    deviceSettings &devSettings = deviceSettings::Instance();

    if( capList.addCapture( captureItem.tagText,
                            currTime.toTime_t(),
                            imageName,
                            devSettings.current()->getDeviceName(),
                            captureItem.pixelsPerMm,
                            captureItem.zoomFactor ) < 0 )
    {
        LOG1("ERROR")
        return;   // Failure warnings generated in the call
    }
}

QString captureMachine::generateClipFileName(const ClipItem_t& clipItem)
{
    return QString ("clip-") + clipItem.strClipNumber;
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
            ClipItem_t clipItem = clipQ.dequeue();
            LOG1(clipItem.strClipNumber)
            mutex.unlock();

            processLoopRecording( clipItem );
        }

        // dequeue and toss the element
        mutex.lock();
        runThreadQ.dequeue();
        mutex.unlock();
    }
}
