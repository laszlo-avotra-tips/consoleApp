/*
 * captureWidget.cpp
 *
 * The capture widget is a compound widget consisting of a scrollable
 * list of recent image captures and a scrollable list of recent
 * OCT Loop recordings. Both types of captures have tags associated
 * with them that can be modified by they user.
 *
 * When in the Live View state, captures have the following behavior:
 *
 *   Single-click: select the item; allow edit of the tag
 *   Double-click: start Review State.  The item is loaded for display
 *                 on the Technician and Physician monitors
 *
 * When in the Review state:
 *
 *   Single-click: The item is loaded for display on the Technician
 *                 and Physician monitors
 *   Double-click: The item is loaded for display on the Technician
 *                 and Physician monitors
 *
 * This file contains the implementation of the widget as well as item
 * delegates to handle drawing of the individual thumbnails.
 *
 * Author: Chris White
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#include "capturewidget.h"
#include "Screens/frontend.h"
#include <QPainter>
#include <QFontMetrics>
#include "defaults.h"
#include "Utility/sessiondatabase.h"
#include "logger.h"

const QSize  ThumbSize( ThumbnailHeight_px + 10, ThumbnailHeight_px + 10 );
//const QColor SelectedItemColor( 143, 185, 224 );
const int    MinOffsetForNumberLabel_px = 10;

/*
 * Constructor
 */
captureItemDelegate::captureItemDelegate( bool rotate, QObject *parent ) :
        QAbstractItemDelegate( parent )
{
    doRotate = rotate;
}

/*
 * sizeHint()
 *
 * Return the appropriate size hint for a capture
 * thumbnail. Handles both rotated and non-rotated
 * modes.
 */
QSize captureItemDelegate::sizeHint( const QStyleOptionViewItem &option,
                                     const QModelIndex & ) const
{
    if( !doRotate )
    {
        return( ThumbSize );
    }
    else
    {
        return( QSize( option.rect.height(), option.rect.width() ) );
    }
}

/*
 * paint()
 *
 * Draw the complete thumbnail including the image, the
 * index overlay, highlights, background and rounded corners.
 */
void captureItemDelegate::paint( QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index ) const
{
    captureItem *item;
    painter->save();
    painter->setFont( QFont( "DinPRO-regular", 20 ) );
    item = index.model()->data( index, Qt::DisplayRole ).value<captureItem *>();

    painter->setPen( QPen( Qt::black, 6 ) );
    QRect baseRect( option.rect.x() + 4,
                    option.rect.y() + 5,
                    option.rect.width() - 10,
                    option.rect.height() - 10 );
    painter->drawRect( baseRect );

    painter->translate( option.rect.x(), option.rect.y() );

    // Get the pixel width of each value so it is written on the image correctly
    QFontMetrics fm = painter->fontMetrics();
    const QString NumberLabel = QString( "%1" ).arg( item->getdbKey() );
    const int Offset_px = MinOffsetForNumberLabel_px + fm.width( NumberLabel );

    if( doRotate )
    {
        // Physician screen
        QMatrix m;
        m.rotate( 90 );

        // The physician preview size is larger than the operators view; scale
        // up the thumbnail to fit without concern for the quality of the preview image
        painter->drawImage( 5, 5, item->loadSectorThumbnail( item->getName() ).scaled( option.rect.height() - 5, option.rect.height() - 5 ).transformed( m ) );
        painter->setPen( QPen( SelectedItemColor, 6 ) );
        painter->rotate( 90 );
        painter->drawText( option.rect.width() - Offset_px, - 10, NumberLabel );
    }
    else
    {
        // Technician screen
        painter->drawImage( 5, 5, item->loadSectorThumbnail( item->getName() ) );
        painter->setPen( QPen( SelectedItemColor, 6 ) );
        painter->drawText( option.rect.width() - Offset_px, option.rect.height() - 10, NumberLabel );
    }
    painter->restore();

    // Highlight the selected item
    if( option.state & QStyle::State_Selected )
    {
        const int PenSize = 1;
        painter->setPen( QPen( SelectedItemColor, PenSize ) );
        QRect borderRect( option.rect.x() + PenSize,
                          option.rect.y() + PenSize,
                          option.rect.width()  - 2 * PenSize,
                          option.rect.height() - 2 * PenSize );
        painter->drawRoundedRect( borderRect, 5, 5 );
    }
}


/***************************************************/

/*
 * constructor
 */
clipItemDelegate::clipItemDelegate( bool rotate, QObject *parent ) :
        QAbstractItemDelegate( parent )
{
    doRotate = rotate;
}

/*
 * sizeHint()
 *
 * Return the appropriate size hint for a capture
 * thumbnail. Handles both rotated and non-rotated
 * modes.
 */
QSize clipItemDelegate::sizeHint( const QStyleOptionViewItem &option,
                                  const QModelIndex & ) const
{
    if( !doRotate )
    {
        return( ThumbSize );
    }
    else
    {
        return( QSize( option.rect.height(), option.rect.width() ) );
    }
}

/*
 * paint()
 *
 * Draw the complete thumbnail including the image, the
 * index overlay, highlights, background and rounded corners.
 */
void clipItemDelegate::paint( QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index ) const
{
    clipItem *item;
    painter->save();
    painter->setFont( QFont( "DinPRO-regular", 20 ) );
    item = index.model()->data( index, Qt::DisplayRole ).value<clipItem *>();

    painter->setPen( QPen( Qt::black, 6 ) );
    QRect baseRect( option.rect.x() + 4,
                    option.rect.y() + 5,
                    option.rect.width() - 10,
                    option.rect.height() - 10 );
    painter->drawRect( baseRect );

    painter->translate( option.rect.x(), option.rect.y() );

    // Get the pixel width of each value so it is written on the image correctly
    QFontMetrics fm = painter->fontMetrics();
    const QString NumberLabel = QString( "%1" ).arg( item->getdbKey() );
    const int Offset_px = MinOffsetForNumberLabel_px + fm.width( NumberLabel  );

    if( !doRotate )
    {
        // Technician screen
        painter->drawImage( 5, 5, item->loadSectorThumbnail( item->getName() ) );
        painter->setPen( QPen( SelectedItemColor, 6 ) );
        painter->drawText( option.rect.width() - Offset_px, option.rect.height() - 10, QString( "%1" ).arg( item->getdbKey() ) );
    }
    painter->restore();

    // Highlight the selected item
    if( option.state & QStyle::State_Selected )
    {
        const int PenSize = 1;
        painter->setPen( QPen( SelectedItemColor, PenSize ) );
        QRect borderRect( option.rect.x() + PenSize,
                          option.rect.y() + PenSize,
                          option.rect.width()  - 2 * PenSize,
                          option.rect.height() - 2 * PenSize );
        painter->drawRoundedRect( borderRect, 5, 5 );
    }
}

/**************************************************/

/*
 * constructor
 */
captureWidget::captureWidget( QWidget *parent ) :
    QWidget( parent ), ui( new Ui::captureWidget )
{
    numCaptures = 0;
    numLoops    = 0;

    displayDecoratedImages  = false;
    isImageReviewInProgress = false;
    isLoopReviewInProgress  = false;

    selectedCaptureItem = nullptr;
    selectedLoopItem    = nullptr;

    ui->setupUi( this );
}

/*
 * init
 *
 * Initialize the lists and set up the default connections; see the description at
 * the top of this file for the connections.
 */
void captureWidget::init( void )
{
    /*
     * Image Captures
     */
    // set up the list for image captures
    captureListModel &capList = captureListModel::Instance();

    ui->capListView->setItemDelegate( new captureItemDelegate() );
    ui->capListView->setModel( &capList );

    connect( ui->capListView, SIGNAL( clicked( QModelIndex ) ), this, SLOT( captureSelected(QModelIndex) ) );
    connect( ui->capListView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );

    // Auto-scroll the list when items are added
    connect( &capList, SIGNAL( rowsInserted( QModelIndex, int, int ) ), ui->capListView, SLOT( updateView( QModelIndex, int, int ) ) );

    // keyboard keys change the selection
    connect( ui->capListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
             this, SLOT(captureSelected(const QModelIndex &)) );

    /*
     * OCT Loops
     */
    // set up the list for clips
    clipListModel &clipList = clipListModel::Instance();

    ui->loopsListView->setItemDelegate( new clipItemDelegate() );
    ui->loopsListView->setModel( &clipList );

    connect( ui->loopsListView, SIGNAL( clicked(QModelIndex) ), this, SLOT( loopSelected(QModelIndex) ) );
    connect( ui->loopsListView, SIGNAL( doubleClicked(QModelIndex) ), this, SLOT( loadLoop(QModelIndex) ) );

    // Auto-scroll the list when items are added
    connect( &clipList, SIGNAL( rowsInserted( QModelIndex, int, int ) ), ui->loopsListView, SLOT( updateView( QModelIndex, int, int ) ) );

    // keyboard keys change the selection
    connect( ui->loopsListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
             this, SLOT(loopSelected(const QModelIndex &)) );

    ui->selectedCaptureLineEdit->installEventFilter( this );
    ui->selectedLoopLineEdit->installEventFilter( this );
}

/*
 * destructor
 */
captureWidget::~captureWidget()
{
    delete ui;
}

// SLOTS and EVENTS

/*
 * displayCapture()
 *
 * Load and display the selected image on the Technician and Physican monitors.
 */
void captureWidget::displayCapture( QModelIndex index )
{
    captureItem *item = index.data( Qt::DisplayRole ).value<captureItem *>();

    if( item )
    {
        float zoomFactor = 1.0;
        if( displayDecoratedImages )
        {
            emit showCapture( item->loadDecoratedImage( item->getName() ), QImage() );
            zoomFactor = selectedCaptureItem->getZoomFactor(); // only apply the saved zoom factor for decorated images
        }
        else
        {
            emit showCapture( item->loadSector( item->getName() ), item->loadWaterfall( item->getName() ) );
        }

        QString str = QString( tr( "REVIEW: %1 (%2)" ) ).arg( item->getTag() ).arg( item->getIdNumber(), 3, 10, QLatin1Char( '0' ) );
        emit sendStatusText( str );

        // update the label to the review device
        emit sendDeviceName( item->getDeviceName() );
        emit displayingCapture();
        emit sendReviewImageCalibrationFactors( selectedCaptureItem->getPixelsPerMm(), zoomFactor );

        if( !isImageReviewInProgress )
        {
            LOG( INFO, "Image Review started" )
            disconnect( ui->capListView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );
            connect(    ui->capListView, SIGNAL( clicked( QModelIndex ) ),       this, SLOT( displayCapture(QModelIndex) ) );

            // keyboard keys change the selection
            connect( ui->capListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                     this, SLOT(displayCapture(const QModelIndex &)) );

            isImageReviewInProgress = true;
        }
    }
}

/*
 * captureSelected()
 *
 * One of the items in the preview list has been selected.
 * Highlight it.
 */
void captureWidget::captureSelected( QModelIndex index )
{
    selectedCaptureItem = index.data( Qt::DisplayRole ).value<captureItem *>();

    ui->selectedCaptureLineEdit->setText( selectedCaptureItem->getTag() );
    emit currentCaptureChanged( index );
}

/*
 * loopSelected()
 *
 * One of the OCT loops in the preview list has been selected.
 * Highlight it.
 */
void captureWidget::loopSelected( QModelIndex index )
{
    selectedLoopItem = index.data( Qt::DisplayRole ).value<clipItem *>();

    ui->selectedLoopLineEdit->setText( selectedLoopItem->getTag() );

//#if LOAD_CLIP_INTO_PREVIEW_BUTTONS
//// TBD: Do not load the clip into the preview window
//    QIcon icon(QPixmap::fromImage(item->loadSector( item->getTag() ).scaled(ui->sectorCaptureButton->size())));
//    ui->sectorCaptureButton->setIcon(icon);

//    icon = QPixmap::fromImage(item->loadWaterfall( item->getTag() ).scaled(ui->waterfallCaptureButton->size()));
//    ui->waterfallCaptureButton->setIcon(icon);

//    emit currentCaptureChanged( index );
//#endif
}

/*
 * loadLoop()
 *
 * Signal that an OCT Loop has been selected and loaded for playback.
 * Do not load a loop if it is less than a minimum recording length (runt loops).
 * Changes the interaction of single- and double-click for the clip list. Also change
 * the keyboard interaction within the clip list.
 */
void captureWidget::loadLoop( QModelIndex index )
{
    clipItem *item = index.data( Qt::DisplayRole ).value<clipItem *>();

    /*
     * Check for runt recordings. Do not load a loop if it is too short.
     */
    if( item->getLength() > MinRecordingLength_s )
    {
        /*
         * Send label information for the loaded loop.
         */
        emit sendLoopFilename( item->getName() );
        emit sendDeviceName(   item->getDeviceName() );
        QString str = QString( tr( "REVIEW: %1 (%2)" ) ).arg( item->getTag(), item->getName().right( 3 ) );
        emit sendStatusText( str );

        /*
         * This is only done when a valid-length loop is loaded.
         */
        if( !isLoopReviewInProgress )
        {
            /*
             * Toggle interaction of single- vs double-click. When a loop is loaded, and we are in the reviewing state,
             * we want single-click action. When we are no-longer in review state, we want to switch back to double-click
             * action.
             */
            disconnect( ui->loopsListView, SIGNAL( doubleClicked(QModelIndex) ), this, SLOT( loadLoop(QModelIndex) ) );
            connect(    ui->loopsListView, SIGNAL( clicked(QModelIndex) ),       this, SLOT( loadLoop(QModelIndex) ) );

            /*
             * Allow keyboard navigation while in the review state.
             */
            connect( ui->loopsListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                     this, SLOT(loadLoop(const QModelIndex &)) );

            /*
             * This is the only location where the isLoopReviewInProgress flag is set.
             */
            isLoopReviewInProgress = true;
        }
    }
    else
    {
        displayWarningMessage( tr( "OCT Loop is too short to be valid. It will not play." ) );
    }
}

/*
 * changeEvent()
 *
 * Boilerplate.
 */
void captureWidget::changeEvent( QEvent *e )
{
    QWidget::changeEvent( e );
//    switch ( e->type() ) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi( this );
//        break;
//    default:
//        break;
//    }
    if(e->type() == QEvent::LanguageChange){
        ui->retranslateUi( this );
    }
}

/*
 * eventFilter
 */
bool captureWidget::eventFilter( QObject *obj, QEvent *event )
{
    /*
     * automatically select the tag text when either of the fields is selected
     */
    if( ( event->type() == QEvent::FocusIn ) &&
        ( ( obj == ui->selectedCaptureLineEdit ) || ( obj == ui->selectedLoopLineEdit ) ) )
    {
        QTimer::singleShot( 0, obj, SLOT( selectAll() ) );
        return false;
    }
    else
    {
        return( QObject::eventFilter( obj, event ) );
    }
}

/*
 * updateCaptureCount
 */
void captureWidget::updateCaptureCount( void )
{
    numCaptures++;
    ui->capturesGroupBox->setTitle( tr( "Images (%1)" ).arg( numCaptures ) );
}

/*
 * updateClipCount
 */
void captureWidget::updateClipCount( void )
{
    numLoops++;
    ui->loopsGroupBox->setTitle( tr( "Loops (%1)" ).arg( numLoops ) );
}

/*
 * reviewStateEnded
 *
 * Reset any modified connections when the Review state was stated
 */
void captureWidget::reviewStateEnded()
{
    if( isImageReviewInProgress )
    {
        isImageReviewInProgress = false;
        connect( ui->capListView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );
        disconnect( ui->capListView, SIGNAL( clicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );
        disconnect( ui->capListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                    this, SLOT(displayCapture(const QModelIndex &)) );

    }

    if( isLoopReviewInProgress )
    {
        isLoopReviewInProgress  = false;
        connect( ui->loopsListView, SIGNAL( doubleClicked(QModelIndex) ), this, SLOT( loadLoop(QModelIndex) ) );
        disconnect( ui->loopsListView, SIGNAL( clicked(QModelIndex) ), this, SLOT( loadLoop(QModelIndex) ) );
        disconnect( ui->loopsListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                    this, SLOT(loadLoop(const QModelIndex &)) );

    }
}

/*
 * on_selectedCaptureLineEdit_editingFinished
 */
void captureWidget::on_selectedCaptureLineEdit_editingFinished()
{
    // Remove any extraneous leading or trailing whitespace before using the text
    ui->selectedCaptureLineEdit->setText( ui->selectedCaptureLineEdit->text().trimmed() );

    if( selectedCaptureItem )
    {
        // Set the tag for the tooltip
        selectedCaptureItem->setTag( ui->selectedCaptureLineEdit->text() );

        // Save the new tag name to the session database
        sessionDatabase &db = sessionDatabase::Instance();
        captureItem *item = ui->capListView->currentIndex().data( Qt::DisplayRole ).value<captureItem *>();
        db.updateCaptureTag( item->getdbKey(), ui->selectedCaptureLineEdit->text() );

        if( isImageReviewInProgress )
        {
            displayCapture( ui->capListView->currentIndex() );
        }
    }

    // Remove focus from the line edit
    ui->selectedCaptureLineEdit->clearFocus();
}

/*
 * on_selectedLoopLineEdit_editingFinished
 */
void captureWidget::on_selectedLoopLineEdit_editingFinished()
{
    // Remove any extraneous leading or trailing whitespace before using the text
    ui->selectedLoopLineEdit->setText( ui->selectedLoopLineEdit->text().trimmed() );

    if( selectedLoopItem )
    {
        // Set the tag for the tooltip
        selectedLoopItem->setTag( ui->selectedLoopLineEdit->text() );

        // Save the new tag name to the session database
        sessionDatabase &db = sessionDatabase::Instance();
        clipItem *item = ui->loopsListView->currentIndex().data( Qt::DisplayRole ).value<clipItem *>();
        db.updateLoopTag( item->getdbKey(), ui->selectedLoopLineEdit->text() );

        // TBD: the loop tag is not currently displayed on the UI; see #1591
        // note: this will cause the loop to restart from teh beginning; that may not
        //       be the desired result
        //if( isLoopReviewInProgress )
        //{
        //    loadLoop( ui->loopsListView->currentIndex() );
        //}
    }

    // Remove focus from the line edit
    ui->selectedLoopLineEdit->clearFocus();
}

/*
 * on_showDecoratedPushButton_toggled
 *
 * Switch to loading the decorated image instead of the raw images. Update
 * the displayed review image if one is showing.
 */
void captureWidget::on_showDecoratedPushButton_toggled( bool checked )
{
    LOG( INFO, QString( "Show Decorated enabled: %1" ).arg( checked ) )
    displayDecoratedImages = checked;
    if( isImageReviewInProgress )
    {
        displayCapture( ui->capListView->currentIndex() );
    }
}

/*
 * updateZoomFactor
 *
 * If reviewing a decorated image, multiply the existing zoom factor.
 * If reviewing an undecorated image, replace the zoom factor.
 */
void captureWidget::updateZoomFactor( float value )
{
    if( isImageReviewInProgress )
    {
        if( displayDecoratedImages )
        {
            selectedCaptureItem->setZoomFactor( selectedCaptureItem->getZoomFactor() * value );
        }
        else
        {
            selectedCaptureItem->setZoomFactor( value );
        }
    }
}
