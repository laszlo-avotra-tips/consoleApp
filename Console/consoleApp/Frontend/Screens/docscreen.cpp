/*
 * docscreen.cpp
 *
 * The secondary window for the OCT application. This window
 * presents the physician with a stripped down view of
 * the GUI, focused on the sector and waterfall.
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#include "docscreen.h"
#include "capturelistview.h"
#include "capturewidget.h"
#include "Utility/captureListModel.h"
#include "Utility/userSettings.h"
#include "logger.h"
#include "defaults.h"

docscreen::docscreen( QWidget *parent )
    : QMainWindow( parent )
{
    captureListModel &capList = captureListModel::Instance();
    ui.setupUi( this );
    proxyModel = new QSortFilterProxyModel( this );

    // Reverse sort the list for the doc screen since
    // it's rotated.
    proxyModel->setSortRole( Qt::UserRole );
    proxyModel->setSourceModel( &capList );
    proxyModel->sort( 0, Qt::DescendingOrder );
    proxyModel->setDynamicSortFilter( true );
    
    ui.previewListview->setItemDelegate( new captureItemDelegate( true ) );

    ui.previewListview->setModel( proxyModel );
    ui.previewListview->hide();
    ui.previewListview->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui.previewListview->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

//lcv
//    ui.timeFieldLabel->setRotation( Qxt::Clockwise );
//    ui.timeFieldLabel->setStyleSheet( "QFrame { color: lightGray }" );

//    ui.statusLabel->setRotation( Qxt::Clockwise );
//    ui.statusLabel->setStyleSheet( "QFrame { color: lightGray }" );

//    ui.recordingLabel->setText( tr( "RECORDING" ) );
//    ui.recordingLabel->setRotation( Qxt::Clockwise );
//    ui.recordingLabel->setStyleSheet( "QFrame { color: yellow }" );
//    ui.recordingLabel->hide();

//    ui.deviceLabel->setRotation( Qxt::Clockwise );
//    ui.deviceLabel->setStyleSheet( "QFrame { color: lightGray }" );
//    ui.deviceLabel->setText( "" );
//    ui.deviceLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

//    ui.zoomFactorLabel->setRotation( Qxt::Clockwise );
//    ui.zoomFactorLabel->setStyleSheet( "QFrame { color: lightGray }" );
//    ui.zoomFactorLabel->setText( "" );
//    ui.zoomFactorLabel->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );

//    ui.catheterViewLabel->setRotation( Qxt::Clockwise );
//    ui.catheterViewLabel->setStyleSheet( "QFrame { color: lightGray }" );
//    ui.catheterViewLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    QPixmap logoPixmap( ":/octConsole/Frontend/Resources/logo-physicianScreen.png" );
    ui.logoLabel->setPixmap( logoPixmap );
}

/*
 * Destructor
 */
docscreen::~docscreen()
{
}

/*
 * setScene()
 *
 * Associate a scene (waterfall + sector) with the view on
 * the doc screen. Set up the rotation, etc. for the portrait
 * orientation.
 */
void docscreen::setScene( QGraphicsScene *scene )
{
    // Scale to fit
    ui.liveGraphicsView->rotate( 90 );

    // Associate the view with the scene
    ui.liveGraphicsView->setScene( scene );
    ui.liveGraphicsView->fitInView( scene->sceneRect(), Qt::KeepAspectRatio );
//#if HIGH_QUALITY_RENDERING
//    // High quality rendering hints
//    ui.liveGraphicsView->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
//#endif
}

/*
 * configureDisplayForReview
 * 
 * Update the screen to indicate Review mode is currently active
 */
void docscreen::configureDisplayForReview( void )
{
    ui.deviceLabel->setStyleSheet( "QFrame { color: yellow }" );
    ui.statusLabel->setStyleSheet( "color: yellow" );
}

/*
 * configureDisplayForLiveView
 * 
 * Update the screen to indicate Live View mode is currently active
 */
void docscreen::configureDisplayForLiveView()
{
    ui.deviceLabel->setStyleSheet( "QFrame { color: lightGray }" );
    ui.statusLabel->setStyleSheet( "QFrame { color: lightGray }" );
}

/*
 * showPreview()
 *
 * Display a labelled preview filmstrip of current captures to
 * allow the physician to choose (verbally) which to review.
 */
void docscreen::showPreview( bool enable )
{
    if( enable )
    {
        proxyModel->invalidate();
        ui.previewListview->show();
        LOG( INFO, "Physician Preview: On" )
    }
    else
    {
        ui.previewListview->hide();
        LOG( INFO, "Physician Preview: Off" )
    }
}

/*
 * updatePreview()
 *
 * Move to whichever capture the technician has selected and
 * center in the filmstrip. TDB: need to update the filmstrip
 * when a new capture is added.
 */
void docscreen::updatePreview( QModelIndex ix )
{
    // if the first item is selected, don't center it and create an open
    // slot in the physician preview (i.e., keep as many images on the preview as possible)
    QModelIndex tmp = ix;
    if( ix.row() < 1 )
    {
        tmp = ix.sibling( 1, 0 );
    }

    ui.previewListview->setCurrentIndex( proxyModel->mapFromSource( ix ) );
    ui.previewListview->scrollTo( proxyModel->mapFromSource( tmp ),
                                  QAbstractItemView::PositionAtCenter );
}


/*
 * setDevice
 *
 * Set the device text on the physician screen. Make the live device label gray and the review device label yellow.
 */
void docscreen::setDeviceName( QString dev )
{
    LOG1(dev)
//lcv    ui.deviceLabel->setText( dev );
}

