#include "captureItemDelegate.h"
#include "Utility/captureListModel.h"
#include "defaults.h"
#include "logger.h"

namespace{
const QSize  ThumbSize( ThumbnailHeight_px + 10, ThumbnailHeight_px + 10 );
const QColor SelectedItemColor( 245, 196, 0 );
const QColor SelectedTextColor( 143, 185, 224 );
const int    MinOffsetForNumberLabel_px = 10;
}

CaptureItemDelegate::CaptureItemDelegate(bool rotate, QObject *parent)
    :QAbstractItemDelegate( parent )
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
QSize CaptureItemDelegate::sizeHint( const QStyleOptionViewItem &option,
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
void CaptureItemDelegate::paint( QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index ) const
{
   captureItem *item;
   painter->save();
   painter->setFont( QFont( "DinPRO-regular", 14 ) );
   item = index.model()->data( index, Qt::DisplayRole ).value<captureItem *>();

   painter->setPen( QPen( Qt::green, 6 ) );
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
       painter->setPen( QPen( SelectedTextColor, 6 ) );
       painter->rotate( 90 );
       painter->drawText( option.rect.width() - Offset_px, - 10, NumberLabel );
   }
   else
   {
       int offset_height = -10;
       int offset_width = Offset_px;
       LOG2(offset_width, offset_height)
       // Technician screen
       painter->drawImage( 5, 5, item->loadSectorThumbnail( item->getName() ) );
       painter->setPen( QPen( SelectedTextColor, 6 ) );
//       painter->drawText( option.rect.width() - Offset_px, option.rect.height() - offset_height, NumberLabel );
       painter->drawText( option.rect.width(), option.rect.height(), NumberLabel );
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

   emit updateLabel();
}
