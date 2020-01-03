/*
 * auxmonitor.cpp
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2016-2018 Avinger, Inc.
 */
#include "auxmonitor.h"
#include "ui_auxmonitor.h"
#include "defaults.h"

/*
 * Constructor
 */
AuxMonitor::AuxMonitor(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    catheterViewLabel.labelName = CatheterView;
    recordingLabel.labelName    = Recording;
    zoomFactorLabel.labelName   = ZoomFactor;
    timeFieldLabel.labelName    = TimeField;
    statusLabel.labelName       = Status;

    catheterViewLabel.uiLabel = ui.catheterViewLabel;
    recordingLabel.uiLabel    = ui.recordingLabel;
    zoomFactorLabel.uiLabel   = ui.zoomFactorLabel;
    timeFieldLabel.uiLabel    = ui.timeFieldLabel;
    statusLabel.uiLabel       = ui.statusLabel;

    catheterViewLabel.cachedText = "";
    recordingLabel.cachedText    = "RECORDING";
    zoomFactorLabel.cachedText   = "";
    timeFieldLabel.cachedText    = "";
    statusLabel.cachedText       = "";

    ui.timeFieldLabel->setStyleSheet(    "QFrame { color: lightGray }" );
    ui.deviceLabel->setStyleSheet(       "QFrame { color: lightGray }" );
    ui.statusLabel->setStyleSheet(       "QFrame { color: lightGray }" );
    ui.catheterViewLabel->setStyleSheet( "QFrame { color: lightGray }" );
    ui.recordingLabel->setStyleSheet(    "QFrame { color: yellow }"    );
    ui.zoomFactorLabel->setStyleSheet(   "QFrame { color: lightGray }" );

    // Set the info box color to the left and right of the sector.
    setStyleSheet( "QGroupBox{ background-color: black }" );

    wm = &WindowManager::Instance();
    connect( wm, SIGNAL(monitorChangesDetected()), this, SLOT(forceResize()) );
    forceResize(); // call on yourself due to the chicken v egg scenario of AuxMonitor and WindowManager
}

/*
 * Destructor
 */
AuxMonitor::~AuxMonitor()
{
}

/*
 * setScene()
 *
 * Associate a scene of the sector with the view on
 * the Aux screen. This should only be the sector portion of the scene,
 * intentionally excluding the waterfall.
 */
void AuxMonitor::setScene( QGraphicsScene *scene )
{
    ui.liveGraphicsView->setScene( scene );
    ui.liveGraphicsView->fitInView( QRectF( 0, 0, SectorWidth_px, SectorHeight_px ), Qt::KeepAspectRatio );
}

/*
 * updateTime
 *
 * Update the timestamp string from Frontend.
 */
void AuxMonitor::updateTime( QString sTime )
{
    ui.timeFieldLabel->setText( sTime );
}

/*
 * setDeviceName
 *
 * Update the device name string from Frontend.
 */
void AuxMonitor::setDeviceName(QString sDevice)
{
    ui.deviceLabel->setText( sDevice );
}

/*
 * getMatrix
 *
 * Getter to access the matrix of liveGraphicsView for Zoom in Frontend.
 */
QMatrix AuxMonitor::getMatrix( void )
{
    return ui.liveGraphicsView->matrix();
}

/*
 * setTransformForZoom
 *
 * Setter to access the Transform of liveGraphicsView for Zoom in Frontend.
 */
void AuxMonitor::setTransformForZoom( QTransform t, bool isZoomed )
{
    ui.liveGraphicsView->setTransform( t );
    if( !isZoomed )
    {
        ui.liveGraphicsView->fitInView( QRectF( 0, 0, SectorWidth_px, SectorHeight_px ), Qt::KeepAspectRatio );
    }
}

/*
 * configureDisplayForReview
 *
 * Update the screen to indicate Review mode is currently active
 */
void AuxMonitor::configureDisplayForReview( void )
{
    ui.deviceLabel->setStyleSheet( "QFrame { color: yellow }" );
    ui.statusLabel->setStyleSheet( "color: yellow" );
}

/*
 * configureDisplayForLiveView
 *
 * Update the screen to indicate Live View mode is currently active
 */
void AuxMonitor::configureDisplayForLiveView()
{
    ui.deviceLabel->setStyleSheet( "QFrame { color: lightGray }" );
    ui.statusLabel->setStyleSheet( "QFrame { color: lightGray }" );
}

/*
 * setText
 *
 * This is a public function to generically set the label text and hide/show.
 * It's advantage over using QLabel::setText() and QLabel::hide()/show() is
 * that it can cache the previous valid string displayed. It also will never hide
 * the label, so when used in a layout, it won't force the layout to resize/rearrange
 * all other widgets.
 */
void AuxMonitor::setText( LabelEnum labelName, bool showText, QString text )
{
    bool validLabel = false;

    T_Label *label;

    switch( labelName )
    {
    case CatheterView:
        label = &catheterViewLabel;
        validLabel = true;
        break;
    case Recording:
        label = &recordingLabel;
        validLabel = true;
        break;
    case ZoomFactor:
        label = &zoomFactorLabel;
        validLabel = true;
        break;
    case TimeField:
        label = &timeFieldLabel;
        validLabel = true;
        break;
    case Status:
        label = &statusLabel;
        validLabel = true;
        break;
    }

    if( validLabel )
    {
        // cache the previously displayed text
        if( !showText && !label->uiLabel->text().isEmpty() )
        {
            label->cachedText = label->uiLabel->text();
        }

        if( showText )
        {
            if( !text.isEmpty() )
            {
                label->uiLabel->setText( text );
            }
            else
            {
                label->uiLabel->setText( label->cachedText );
            }
        }
        else
        {
            label->uiLabel->setText( "" );
        }
    }
}

/*
 * liveGraphicsViewCenterOn
 */
void AuxMonitor::liveGraphicsViewCenterOn(qreal x, qreal y)
{
    ui.liveGraphicsView->centerOn( x, y );
}

/*
 * getLiveGraphicsViewHorizontalScrollBar
 */
const QScrollBar *AuxMonitor::getLiveGraphicsViewHorizontalScrollBar()
{
    return ui.liveGraphicsView->horizontalScrollBar();
}

/*
 * getLiveGraphicsViewVerticalScrollBar
 */
const QScrollBar *AuxMonitor::getLiveGraphicsViewVerticalScrollBar()
{
    return ui.liveGraphicsView->verticalScrollBar();
}

/*
 * setLiveGraphicsViewHorizontalScrollBar
 */
void AuxMonitor::setLiveGraphicsViewHorizontalScrollBar(int val)
{
    ui.liveGraphicsView->horizontalScrollBar()->setValue( val );
}

/*
 * setLiveGraphicsViewVerticalScrollBar
 */
void AuxMonitor::setLiveGraphicsViewVerticalScrollBar(int val)
{
    ui.liveGraphicsView->verticalScrollBar()->setValue( val );
}

/*
 * forceResize
 *
 * Signalled by WindowManager emit of monitorChangeDetected().
 * - make scene large and square
 * - determine position of text if it should be in left & right info boxes or just on right
 *   due to aspect ratio.
 */
void AuxMonitor::forceResize()
{
    if( wm->isAuxMonPresent() )
    {
        optimizeSceneSize();
        testAndMoveInfo();
        showFullScreen();
    }
}

/*
 * optimizeSceneSize
 *
 * Manually set the scene size since we can't force resizeEvent().
 */
void AuxMonitor::optimizeSceneSize()
{
    ui.liveGraphicsView->setFixedWidth( wm->getAuxilliaryDisplayGeometry().height() );
    if( ui.liveGraphicsView->scene() )
    {
        // always show square scene
        QRectF squareRect( ui.liveGraphicsView->scene()->sceneRect() );
        squareRect.setHeight( squareRect.width() );
        ui.liveGraphicsView->fitInView( squareRect, Qt::KeepAspectRatio );
    }
}

/*
 * testAndMoveInfo
 *
 * Depending on the aspect ratio of the display, determine if there is room for two info bars on the side of Sector,
 * or if there is only room for one. Move text to the proper position in either case.
 */
void AuxMonitor::testAndMoveInfo()
{
    WindowManager &windowManager = WindowManager::Instance();
    /*
     * 1.59 is the threshold for aspect ratio.
     * If the ratio is below 1.59 (square monitor), then we move text into the right info box and hide the left box.
     * If the ratio is above 1.59 (wide monitor), then we move text into a left and right box.
     */
    if( ( windowManager.getAuxilliaryDisplayGeometry().width() / float(windowManager.getAuxilliaryDisplayGeometry().height() ) ) < 1.59f )
    {
        QVBoxLayout *vbLayout = qobject_cast<QVBoxLayout*>( ui.infoGroupBox_2->layout() );
        vbLayout->insertWidget( 0, ui.statusLabel );        // desired placement at index 0
        vbLayout->insertWidget( 2, ui.deviceLabel );        // desired placement at index 2
        vbLayout->insertWidget( 3, ui.catheterViewLabel );  // desired placement at index 3
        ui.statusLabel->setAlignment(       Qt::AlignRight | Qt::AlignVCenter );
        ui.deviceLabel->setAlignment(       Qt::AlignRight | Qt::AlignVCenter );
        ui.catheterViewLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        ui.infoGroupBox_2->setLayout( vbLayout );
        ui.infoGroupBox->hide();
        ui.infoGroupBox_2->update();
    }
    else
    {
        // don't know if we need to move items from right box to left box
        QLayout *tmpLayout = ui.infoGroupBox->layout();
        if( tmpLayout->count() < 3 ) // condition where all are in right layout
        {
            QVBoxLayout *vbLayout = qobject_cast<QVBoxLayout*>( ui.infoGroupBox->layout() );
            vbLayout->insertWidget( 0, ui.statusLabel );        // desired placement at index 0
            vbLayout->insertWidget( 1, ui.deviceLabel );        // desired placement at index 1
            vbLayout->insertWidget( 2, ui.catheterViewLabel );  // desired placement at index 2
            ui.statusLabel->setAlignment(       Qt::AlignLeft | Qt::AlignVCenter );
            ui.deviceLabel->setAlignment(       Qt::AlignLeft | Qt::AlignVCenter );
            ui.catheterViewLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
            ui.infoGroupBox->setLayout( vbLayout );
            ui.infoGroupBox->show();
            ui.infoGroupBox->update();
            ui.infoGroupBox_2->update();
        }
    }
}

/*
 * resizeEvent
 *
 * Implemented to adjust the graphicsview to be square.
 * Reference: http://test-qtcentre.wysota.org/threads/68242-Trying-to-use-Layouts-and-QGraphicsView-to-resize-widget-for-any-monitor-resolution
 */
void AuxMonitor::resizeEvent( QResizeEvent *event )
{
    QMainWindow::resizeEvent( event );
    ui.liveGraphicsView->setFixedWidth( event->size().height() );
    if( ui.liveGraphicsView->scene() )
    {
        // always show square scene
        QRectF squareRect( ui.liveGraphicsView->scene()->sceneRect() );
        squareRect.setHeight( squareRect.width() );
        ui.liveGraphicsView->fitInView( squareRect, Qt::KeepAspectRatio );
    }
}
