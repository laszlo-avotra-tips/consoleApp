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

void CaptureItemDelegate::paint( QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index ) const
{
   painter->save();
   painter->setFont( QFont( "DinPRO-regular", 12 ) );

   const captureListModel& capList = captureListModel::Instance();
   const int rowNum = index.row() + capList.getRowOffset();



   auto itemList = capList.getAllItems();

   captureItem * item = itemList.at(rowNum);

   if(item){

   painter->setPen( QPen( Qt::black, 6 ) );
   QRect baseRect( option.rect.x() + 4,
                   option.rect.y() + 5,
                   option.rect.width() - 10,
                   option.rect.height() - 10 );

//   LOG2(option.rect.width(), option.rect.height())
//   LOG2(baseRect.width(), baseRect.height())
//   LOG2(option.rect.x(), option.rect.y())

   painter->drawRect( baseRect );

   painter->translate( option.rect.x(), option.rect.y() );

   // Get the pixel width of each value so it is written on the image correctly
   QFontMetrics fm = painter->fontMetrics();
   const QString NumberLabel = QString( "%1" ).arg( item->getdbKey() );
   const int Offset_px = MinOffsetForNumberLabel_px + fm.width( NumberLabel );
   QImage tmi = item->loadSectorThumbnail( item->getName());
   painter->drawImage( 5, 5, tmi.scaled(160,160));
   painter->setPen( QPen( SelectedTextColor, 6 ) );

   painter->drawText( option.rect.width() - Offset_px, option.rect.height() - 4, NumberLabel );
   painter->restore();

   // Highlight the selected item
//   if( option.state & QStyle::State_Selected )
   if(capList.getIsSelected() && rowNum == capList.getSelectedRow())
   {
       const int PenSize = 2;
       painter->setPen( QPen( SelectedItemColor, PenSize ) );
       QRect borderRect( option.rect.x() + PenSize,
                         option.rect.y() + PenSize,
                         option.rect.width()  - 2 * PenSize,
                         option.rect.height() - 2 * PenSize );
       painter->drawRoundedRect( borderRect, 5, 5 );
   }

   emit updateLabel();
   }
}

void CaptureItemDelegate::handleDisplayOffset(int dpo)
{
    m_itemOffset = dpo;
    LOG1(dpo)
}
