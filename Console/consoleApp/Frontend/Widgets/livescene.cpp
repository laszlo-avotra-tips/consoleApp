/*
 * livescene.cpp
 *
 * Implements the QGraphicsScene derived object that handles all live data presentation 
 * including the sector and any indicators that go along with them.
 * 
 * This object also consumes the line data directly and adds it to the relevant
 * display objects.
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#include "livescene.h"
#include "defaults.h"
#include <QImage>
#include "Utility/userSettings.h"
#include "depthsetting.h"
#include "profiler.h"
#include <QDateTime>
#include <QPainter>
#include "logger.h"
#include <QApplication>
#include "Utility/userSettings.h"
#include "rotationIndicatorOverlay.h"


QString timestampToString( unsigned long ts );

// SceneWidth = sector drawing area. At a minimum, this needs to accommodate ( the 512 pixel radius +
// the catheter radius ) * 2.
const int SceneWidth( SectorWidth_px );
const int SceneHeight( SectorHeight_px );
const int ScreenRefreshRate_ms( 66 );

const int TextHeight( 50 ); // Height of text rendered for video info
const int TextWidth( int(SectorWidth_px / 4.25 ) ); // Width of text rendered for video info, sized to avoid sector edge
const int TextInterlineSpacing( 5 ); // 5 pixels between lines of text
const int LinesOfText( 3 );

// Advance/rewind percents
const double ClipStep_percent( 10.0 );

// Video state update rate
const int ClipUpdateRate_ms( 100 );

/*
 * Constructor
 */
liveScene::liveScene( QObject *parent )
    : QGraphicsScene( 0, 0, SceneWidth, SceneHeight, parent )
{
//	qDebug() << "***** liveScene constructor";
    //lcv
    LOG2(ClipStep_percent, ClipUpdateRate_ms)
    // Items for display
    sector = new sectorItem();
    sector->setData( SectorItemKey, "sector" );
    addItem( sector );

    // Background image rendering for movies
    videoSector = new sectorItem();
    videoSector->setVideoOnly();

    doPaint = false;

    sector->setZValue( 1.0 );
    sector->setPos( 0, 0 );
    sector->clearRotationFlag();

    overlays = new overlayItem( sector );
    this->addItem( overlays );
    overlays->setPos( 0, 0 );
    overlays->setZValue( 100.0 );
    overlays->setVisible( true );

    infoMessageItem = nullptr;
    refreshTimer = new QTimer();
    connect( refreshTimer, SIGNAL( timeout() ), this, SLOT( refresh() ) );
    refreshTimer->start( ScreenRefreshRate_ms);//lcv

    infoRenderTimer = new QTimer();
    connect( infoRenderTimer, SIGNAL( timeout() ), this, SLOT( generateClipInfo() ) );
    infoRenderTimer->start( 100 );
//lcv #pragma message("magic number here, fix")

    reviewing              = false;
    zoomFactor             = 1.0;
    mouseRotationEnabled   = true;
    isAnnotateMode         = false;
    isMeasureMode          = false;
    cachedCalibrationScale = -1; // default value

    reviewSector    = nullptr;

    connect( this, SIGNAL(captureAll(QImage,QImage,QString,unsigned int,int,float)), &capturer, SLOT(imageCapture(QImage,QImage,QString,uint,int,float)) );
    connect( this, SIGNAL(clipCapture(QImage,QString,unsigned int)), &capturer, SLOT(clipCapture(QImage,QString,unsigned int)) );

    connect( &capturer, SIGNAL(warning( QString ) ),     this, SIGNAL(sendWarning( QString ) ) );
    connect( &capturer, SIGNAL(sendFileToKey(QString)),  this, SIGNAL(sendFileToKey(QString)));
    connect( &capturer, SIGNAL(sendCaptureTag(QString)), this, SIGNAL(sendCaptureTag(QString)));
    connect( &capturer, SIGNAL(updateCaptureCount()),    this, SIGNAL(updateCaptureCount()) );
    connect( &capturer, SIGNAL(updateClipCount()),       this, SIGNAL(updateClipCount()) );

    /*
     * Default gray scale palette, linear transformation
     */
    grayScalePalette.resize( ColorTableSize );
    for ( int i = 0; i < ColorTableSize; i++ )
    {
        grayScalePalette[ i ] =  qRgb( i, i, i );
    }

    currColorMap = grayScalePalette;

    /*
     * set up the color map for the component images
     */
    sector->updateColorMap( currColorMap );

    infoRenderBuffer = nullptr;
    infoImage = nullptr;

//    clipPlayer = new videoDecoderItem();
//    addItem( clipPlayer );
//    clipPlayer->setTickInterval( ClipUpdateRate_ms );
//    clipPlayer->setZValue( 3.0 );
//    clipPlayer->setPos( 0, 0 );
//    clipPlayer->hide();

//    connect( clipPlayer, SIGNAL( finished() ), this, SIGNAL( endOfFile() ) );
//    connect( clipPlayer, SIGNAL( totalTimeChanged( qint64 ) ), this, SIGNAL( clipLengthChanged(qint64) ) );
//    connect( clipPlayer, SIGNAL( tick( qint64 ) ), this, SIGNAL( videoTick( qint64 ) ) );

    /*
     * Load direction indicator images for both directions and stopped.
     */
    clockwiseRingImage        = QImage( ":/octConsole/Frontend/Resources/clockwiseArrow.png" );
    counterClockwiseRingImage = QImage( ":/octConsole/Frontend/Resources/counterClockwiseArrow.png" );
    stoppedRingImage          = QImage( clockwiseRingImage.height(), clockwiseRingImage.height(), QImage::Format_Indexed8 );

    clockwiseRingImage        = clockwiseRingImage.convertToFormat( QImage::Format_Indexed8, grayScalePalette );
    counterClockwiseRingImage = counterClockwiseRingImage.convertToFormat( QImage::Format_Indexed8, grayScalePalette );
    stoppedRingImage          = stoppedRingImage.convertToFormat( QImage::Format_Indexed8, grayScalePalette );

    dirRingImageHeight_px     = clockwiseRingImage.height();
    rotationDirection         = directionTracker::Stopped;
    stoppedRingImage.fill( Qt::black );

    activeIndicatorImage      = QImage( ":/octConsole/Frontend/Resources/activeIndicator.png" );
    activeIndicatorRingImage  = activeIndicatorImage.convertToFormat( QImage::Format_Indexed8, grayScalePalette );
    passiveIndicatorImage     = QImage( ":/octConsole/Frontend/Resources/passiveIndicator.png" );
    passiveIndicatorRingImage = passiveIndicatorImage.convertToFormat( QImage::Format_Indexed8, grayScalePalette );

    rotationIndicatorOverlayItem = new RotationIndicatorOverlay(this);

}

/*
 * Destructor
 */
liveScene::~liveScene()
{
    if( sector )
    {
        delete sector;
    }

    if( refreshTimer )
    {
        delete refreshTimer;
    }
    if( annotateOverlayItem )
    {
        delete annotateOverlayItem;
    }

    if( areaOverlayItem )
    {
        delete areaOverlayItem;
    }

    delete rotationIndicatorOverlayItem;
}

/*
 * setAnnotateMode
 */
void liveScene::setAnnotateMode( bool state, QColor color )
{
    isAnnotateMode        = state;
    isAnnotateModeEnabled = state;
    mouseRotationEnabled  = !state;

    if( isAnnotateMode  )
    {
        annotateOverlayItem = new AnnotateOverlay();
        this->addItem( annotateOverlayItem );
        annotateOverlayItem->setZValue( 116.0 );
        annotateOverlayItem->setColor( color );
    }
    else
    {
        if( annotateOverlayItem )
        {
            this->removeItem( annotateOverlayItem );
            delete annotateOverlayItem;
            annotateOverlayItem = nullptr;
        }
    }
}


// Slots

/*
 * refresh
 *
 * Allow the sector to complete their rendering, if anything
 * has actually changed in the last interval
 */
void liveScene::refresh( void )
{
    if( doPaint )
    {
        doPaint = false;
        if(deviceSettings::Instance().getIsSimulation()){
            sector->paintSector( force );
            videoSector->paintSector( force );
        }
        overlays->render();
    }
    if(deviceSettings::Instance().getIsSimulation()){
        update();
    }
}

bool liveScene::getIsRotationIndicatorOverlayItemEnabled() const
{
    return isRotationIndicatorOverlayItemEnabled;
}

void liveScene::setIsRotationIndicatorOverlayItemEnabled(bool value)
{
    isRotationIndicatorOverlayItemEnabled = value;
}

void liveScene::setActive()
{

    if(isRotationIndicatorOverlayItemEnabled){
        if(!rotationIndicatorOverlayItem){
            rotationIndicatorOverlayItem = new RotationIndicatorOverlay(this);
        }
        rotationIndicatorOverlayItem->addItem();
        rotationIndicatorOverlayItem->setText(" ACTIVE");
    }
}

void liveScene::setPassive()
{

    if(isRotationIndicatorOverlayItemEnabled){
        if(!rotationIndicatorOverlayItem){
            rotationIndicatorOverlayItem = new RotationIndicatorOverlay(this);
        }
        rotationIndicatorOverlayItem->addItem();
        rotationIndicatorOverlayItem->setText("PASSIVE");
    }
}

void liveScene::setIdle()
{

    if(isRotationIndicatorOverlayItemEnabled){
        if(!rotationIndicatorOverlayItem){
            rotationIndicatorOverlayItem = new RotationIndicatorOverlay(this);
        }
        rotationIndicatorOverlayItem->removeItem();
        update();
    }
}

/*
 * showReview()
 *
 * Given a sector image, present the image on the scene
 * over the live view.
 */
void liveScene::showReview( const QImage & sec )
{
    if( reviewing )
    {
        removeItem( reviewSector );
        delete reviewSector;
        reviewSector    = nullptr;
    }

    // turn off overlays during review
    overlays->setVisible( false );

    reviewing = true;

    reviewSector    = new QGraphicsPixmapItem( QPixmap::fromImage( sec ) );

    reviewSector->setZValue( 5.0 );
    reviewSector->setPos( 0, 0 );
    addItem( reviewSector );
}

/*
 * resetRotationCounter()
 *
 * Tell the sector item to restart its full rotation counter.
 */
void liveScene::resetRotationCounter( )
{
    sector->clearRotationFlag();
}

void liveScene::handleReticleBrightnessChanged()
{
    auto value = userSettings::Instance().reticleBrightness();
    LOG1(value)
    sector->setReticleBrightness( value );
}

/*
 * addScanFrame
 * Given a shared pointer to an OCT frame,
 * hand it off to all interested display items
 * and schedule a display update at the next interval.
 */
void liveScene::addScanFrame( QSharedPointer<scanframe> &data )
{
    // Pass off to the sector
    sector->addFrame( data );
    videoSector->addFrame( data );

    // Notify anyone interested if a full rotation has
    // taken place. Used, by the lag correction process
    // for example.
    if( sector->fullRotationCompleted() )
    {
        emit fullRotation();
        sector->clearRotationFlag();
    }

    doPaint = true;
}

// SLOTS

/*
 * capture
 *
 * Do an image capture of the sector,
 * updating the capture database as necessary and notifying
 * interested parties that a new capture is loaded.
 */
void liveScene::captureDi( QImage decoratedImage, QString tagText )
{
    /*
     * Render the sector image,
     * then pass of to the capturer to write to
     * disk.
     */
    QImage secImage;
    {
        secImage = sector->freeze();
    }

    depthSetting &ds = depthSetting::Instance();
    int pixelsPerMm = ds.getPixelsPerMm();
    qDebug() << __FUNCTION__ << "." <<  __LINE__ << ": pixelsPerMm=" << pixelsPerMm;

    deviceSettings &dev = deviceSettings::Instance();
    if( dev.current()->getMeaurementVersion() != SupportedMeasurementVersion )
    {
        pixelsPerMm = -1; // assign to -1 meaning the feature is disabled.
    }

    /*
     * Perform the capture. Allow the capture text to be translated.
     */
    {
        emit captureAll( decoratedImage, secImage, tr( tagText.toLocal8Bit().constData() ), sector->getFrozenTimestamp(), pixelsPerMm, zoomFactor );
    }
}

/*
 * setClipForPlayback()
 *
 * The user has selected a clip, get everything queued up.
 */
void liveScene::setClipForPlayback( QString name )
{
    caseInfo &info = caseInfo::Instance();
    clipPath = info.getClipsDir() + "/" + name + LoopVideoExtension;

    qDebug() << "Player loading: " << clipPath;
//    clipPlayer->load( clipPath );
//    QRectF clipSize = clipPlayer->boundingRect();
//    clipPlayer->setScale( (double)SectorWidth_px / (double)clipSize.width() );
//    if( clipPlayer->state() == videoDecoderItem::ErrorState )
//    {
//        LOG( DEBUG, "Player error: videoDecoderItem::ErrorState" );
//        qDebug() << "Player error: videoDecoderItem::ErrorState";
//    }

//    // hide overlays
//    clipPlayer->show();
    overlays->setVisible( false );
    reviewing = true;
}

/*
 * showMessage()
 *
 * Display a message on the doctor view.
 */
void liveScene::showMessage( QString message )
{
    if( infoMessageItem )
    {
        removeItem( infoMessageItem );
        delete infoMessageItem;
        infoMessageItem = nullptr;
    }

    infoMessageItem = new QGraphicsTextItem( );
    infoMessageItem->setPos( 0, 0 );
    infoMessageItem->setPlainText( message );
    infoMessageItem->setFont( QFont( "DinPro-Medium", 18 ) );
    infoMessageItem->setDefaultTextColor( Qt::green );
    addItem( infoMessageItem );
    infoMessageItem->show();

    // Value below 1.0 does not appear above sector--above 1.0 may hide sector.
    infoMessageItem->setZValue( 1.0 );
}

/*
 * hideAnnotations
 */
void liveScene::hideAnnotations()
{
    if( annotateOverlayItem )
    {
        annotateOverlayItem->hide();
    }
}

/*
 * showAnnotations
 */
void liveScene::showAnnotations()
{
    if( annotateOverlayItem )
    {
        annotateOverlayItem->show();
    }
}

/*
 * startPlayback
 */
void liveScene::startPlayback()
{
//    clipPlayer->play();
}

/*
 * pausePlayback
 */
void liveScene::pausePlayback()
{
//    clipPlayer->pause();
}

/*
 * advancePlayback
 */
void liveScene::advancePlayback()
{
//    qint64 advanceTime = clipPlayer->currentTime() + (clipPlayer->totalTime() / ClipStep_percent);
//    if( advanceTime > clipPlayer->totalTime() )
//    {
//        clipPlayer->seek( clipPlayer->totalTime() );
//    }
//    else
//    {
//        clipPlayer->seek( advanceTime );
//    }
}

/*
 * rewindPlayback
 */
void liveScene::rewindPlayback()
{
//    qint64 rewindTime = clipPlayer->currentTime() - (clipPlayer->totalTime() / ClipStep_percent);
//    if( rewindTime < 0 )
//    {
//        clipPlayer->seek( 0 );
//    }
//    else
//    {
//        clipPlayer->seek( rewindTime );
//    }
}

/*
 * mousePressEvent()
 *
 * Handle mouse button presses
 */
void liveScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if( zoomFactor != 1.0f ) // no zoom
    {
        // capture the event; allows image review to pan and zoom
    } 
    else if( reviewing && !isMeasureMode )
    {
        dismissReviewImages();
    }
    else if( isMeasureMode )
    {
        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mousePressEvent(event);
    }
    else if( isAnnotateMode )
    {
        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mousePressEvent(event);
    }
    else
    {
        if( mouseRotationEnabled )
        {
            LOG( INFO, "Sector rotate" )
            // Grab the sector
            qApp->setOverrideCursor( Qt::ClosedHandCursor );

            // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
            QGraphicsScene::mousePressEvent(event);
        }
    }
    update();
}

/*
 * mouseMoveEvent()
 *
 */
void liveScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if( isMeasureMode )
    {
        qApp->setOverrideCursor( Qt::CrossCursor );

        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mouseMoveEvent( event );
    }
    else if( isAnnotateMode ) /// TBD
    {
        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mouseMoveEvent( event );
    }
    else if ( mouseRotationEnabled )
    {
        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mouseMoveEvent( event );
    }
    else
    {
        qApp->setOverrideCursor( Qt::OpenHandCursor );
    }
}

/*
 * mouseReleaseEvent()
 *
 * Handle mouse button releases
 */
void liveScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if( mouseRotationEnabled )
    {
        qApp->restoreOverrideCursor();

        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mouseReleaseEvent(event);

        // Update the video-only rendering for the current roation on the screen
        videoSector->setDisplayAngle( sector->getDisplayAngle() );

        emit sendDisplayAngle( float(sector->getDisplayAngle()) );
    }
    else if( isAnnotateMode )
    {
        // Ignore the event at the scene level and pass it on to the QGraphicsItem under the mouse
        QGraphicsScene::mouseReleaseEvent(event);
    }
    update();
}

/*
 * captureClip
 *
 * Save the sector from the start of the clip
 */
void liveScene::captureClip( QString strIter )
{
    /*
     * Render the sector images,
     * then pass of to the capturer to write to
     * disk.
     */
    QImage secImage = sector->freeze();

    // Perform the capture. Allow the capture text to be translated.
    emit clipCapture( secImage, strIter, sector->getFrozenTimestamp() );
}

/*
 * dismissReviewImages
 *
 * Method to programmatically dismiss review images
 */
void liveScene::dismissReviewImages( void )
{
    // Dismiss the images and clear messages and states
    if( reviewSector )
    {
        LOG( INFO, "Image Review stopped" )
        removeItem( reviewSector );
        delete reviewSector;
        reviewSector = nullptr;
    }

    reviewing = false;

    // turn overlays back on after review.
    overlays->setVisible( true );

    emit showCurrentDeviceLabel();
    emit sendStatusText( tr( "LIVE" ) );
    emit reviewImageDismissed();
}

/*
 * timestampToString()
 *
 * Painfully, all of the Qt time to string routines
 * take into account the time zone of the machine
 * current running the software. We want to ignore that
 * and use the already baked time from the file.
 */
QString timestampToString( unsigned long ts )
{
    /*
     * Sigh, have to do this ourselves it seems. Compute the
     * hour, minutes, seconds since 00:00:00 Jan 1 1970
     */
    double hours = ts / 3600.0;
    int    wholehours( int(floor(hours)) );
    double frachours = hours - wholehours;

    double minutes = frachours * 60.0;
    int    wholemins = int(floor( minutes ));
    double fracmins = minutes - wholemins;
    int    seconds = int(fracmins * 60.0);

    return QString( "%1:%2:%3" ).arg( wholehours % 24, 2, 10, QChar( '0' ) ).arg( wholemins % 60, 2, 10, QChar( '0' ) ).arg( seconds, 2, 10, QChar( '0' ) );
}

/*
 * generateClipinfo()
 *
 * Draw the time and some case data at the bottom of the frame.
 */
void liveScene::generateClipInfo()
{
    QMutexLocker lock( &infoRenderLock );

    QString timeStr = QDateTime::currentDateTime().toString( "hh:mm:ss" );
    deviceSettings &devInfo = deviceSettings::Instance();

    QString timeString( QString( "%1" ).arg( timeStr, 8 ) );
    QString caseString( QString( devInfo.getCurrentDeviceName() ) );

    size_t textSize = ( TextHeight * LinesOfText + TextInterlineSpacing ) * SectorWidth_px;
    if ( !infoImage )
    {
        infoImage = new QImage( SectorWidth_px, TextHeight * LinesOfText + TextInterlineSpacing, QImage::Format_RGB32 );
    }

    if ( infoLogoImage.isNull() )
    {
        infoLogoImage = QImage( ":/octConsole/Frontend/Resources/logo-video.png" );
        infoLogoImage = infoLogoImage.convertToFormat( QImage::Format_Indexed8, grayScalePalette );
    }

    if ( !infoRenderBuffer )
    {
        infoRenderBuffer = static_cast<char *>(malloc( textSize ) );
    }
    QPainter p( infoImage );
    p.fillRect(0, 0, SectorWidth_px, TextHeight * LinesOfText + TextInterlineSpacing, Qt::black );
    p.setPen( QPen( Qt::white ) );

    p.setFont( QFont( "DinPro-regular", 20 ) );
    p.drawText( 0, TextHeight - 8, timeString );
    p.drawText( 0, TextHeight + 25, caseString );

    p.end();

    // copy info to infoBuffer to be rendered by applyClipInfotoBuffer()
    memcpy( infoRenderBuffer,
            infoImage->convertToFormat( QImage::Format_Indexed8, grayScalePalette ).bits(),
            textSize );
}

/*
 * applyClipInfoToBuffer
 */
void liveScene::applyClipInfoToBuffer( char *buffer )
{
    uchar *logoBits = infoLogoImage.bits();
    QMutexLocker lock( &infoRenderLock );

    /*
     * Add device and time.
     */
    for( int i = 0; i < ( TextHeight * LinesOfText ) + TextInterlineSpacing; i++ )
    {
        memcpy( buffer + ( i * SectorWidth_px ), infoRenderBuffer + i * SectorWidth_px, TextWidth );
    }

    /*
     * Something strange is happening with the conversion of the logo-video.png
     * It is 163 pixels wide, and reports that, but scanning through it in the usual
     * manner is producing a one pixel shift per line, as if it were really 164 pixels wide.
     * Hence the "+ 1" after the .width() call here. Maybe someone can explain this to me.
     */
    for( int i = 0; i < infoLogoImage.height(); i++ )
    {
        memcpy( buffer + ( i * SectorWidth_px ) + ( SectorWidth_px - infoLogoImage.width() ),
                logoBits + ( i * ( infoLogoImage.width() + 1 ) ),
                size_t(infoLogoImage.width() ) );
    }
}

/*
 * updateGrayScaleMap
 *
 * update the color map in grayscale only
 */
void liveScene::updateGrayScaleMap( QVector<unsigned char> map )
{
    for ( int i = 0; i < ColorTableSize; i++ )
    {
        unsigned char val = map[ i ];
        currColorMap[ i ] =  qRgb( val, val, val );
    }

    sector->updateColorMap( currColorMap );
}

/*
 * loadColormap
 * 
 * Load the colormap specified by colormapFile. If the file doesn't exist, it produces a message to the user
 * and continues working with the previous setting.
 */
void liveScene::loadColormap( QString colormapFile )
{
    QFile *input = new QFile( colormapFile );
    LOG1(colormapFile)
    if( !input )
    {
        // warn and do not update the colormap
        emit sendWarning( tr( "Could not create QFile to load colormap data" ) );
        return;
    }

    if( !input->open( QIODevice::ReadOnly ) )
    {
        // warn and do not update the colormap
        QString errString = tr( "Could not open " ) + colormapFile;
        emit sendWarning( errString );
        return;
    }

    QTextStream in( input );
    QString currLine = "";

    int r, g, b;

    // Load the data into the arrays
    for( int i = 0; i < 256; i++ )
    {
        currLine = in.readLine();
        r = currLine.section( ",", 0, 0 ).toInt();
        g = currLine.section( ",", 1, 1 ).toInt();
        b = currLine.section( ",", 2, 2 ).toInt();

        currColorMap[ i ] =  qRgb( r, g, b );
    }

    sector->updateColorMap( currColorMap );

    // free the pointer.  nullptr check done above.
    delete input;
}

/*
 * loadColorModeGray
 */
void liveScene::loadColorModeGray()
{
    LOG1(SystemDir)
    loadColormap( SystemDir + "/colormaps/" + "gray.csv" );
}

/*
 * loadColorModeSepia
 */
void liveScene::loadColorModeSepia()
{
    LOG1(SystemDir)
    loadColormap( SystemDir + "/colormaps/" + "sepia.csv" );
}

/*
 * setMeasureModeArea
 */
void liveScene::setMeasureModeArea( bool state, QColor color )
{
    isMeasureMode = state;
    isMeasurementEnabled = state;

    if( isMeasureMode )
    {
        areaOverlayItem = new AreaMeasurementOverlay( nullptr );
        this->addItem( areaOverlayItem );
        areaOverlayItem->setZValue( 6.0 );
        areaOverlayItem->setColor( color );
        areaOverlayItem->setCalibrationScale( cachedCalibrationScale );

        rotationIndicatorOverlayItem = new RotationIndicatorOverlay(this);
    }
    else
    {
        if( areaOverlayItem )
        {
            this->removeItem( areaOverlayItem );
            delete areaOverlayItem;
            areaOverlayItem = nullptr;
        }
        if(rotationIndicatorOverlayItem){
//            this->removeItem( rotationIndicatorOverlayItem );
            delete rotationIndicatorOverlayItem;
//            rotationIndicatorOverlayItem = nullptr;
        }
    }
}

/*
 * setCalibrationScale
 *
 * This was called through the gui when a capture has been selected for review.
 * The pixelsPerMm and zoomFactor are stored with the capture, and these create
 * the calibration scale that is passed to the measurement overlay for
 * converting pixel lengths to mm scale.
 *
 * Since this slot is called when a capture is called up for review, it is
 * most-likely called before the measurement overlay is created; therefore,
 * it is necessary to cache the scale value and apply it when measurements are
 * enabled.
 */
void liveScene::setCalibrationScale( int pixelsPerMm, float zoomFactorValue )
{
    cachedCalibrationScale = pixelsPerMm;
    cachedCalibrationScale = int( cachedCalibrationScale * zoomFactorValue);
    if( areaOverlayItem != nullptr )
    {
        areaOverlayItem->setCalibrationScale( cachedCalibrationScale );
    }
}
