#include <QPainter>
#include <QStyle>
#include <QStyleOptionHeader>
#include <QStyleOptionViewItem>
#include "avingerstyle.h"

void avingerStyle::drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget ) const
{
    switch ( element ) {

    case CE_Header:
        {
            const QStyleOptionHeader *headerOption = qstyleoption_cast<const QStyleOptionHeader *>(option);
            if ( headerOption ) {
                painter->setFont( QFont( "DinPro-Medium", 13) );
                painter->setPen( QColor( Qt::black ) );
                painter->drawText( headerOption->rect.adjusted( 1, 1, 1, 1 ), headerOption->text );
                painter->setPen( QColor( Qt::white ) );
                painter->drawText( headerOption->rect, headerOption->text );

            }
        }
        break;

    case CE_ItemViewItem:
        {
            const QStyleOptionViewItem *itemOption = qstyleoption_cast<const QStyleOptionViewItem *>(option);
            if ( itemOption ) {
                painter->setPen( QColor( qRgb( 0x90, 0x90, 0x90 ) ) );
                painter->drawLine(itemOption->rect.bottomLeft(), itemOption->rect.bottomRight());
            }
            QProxyStyle::drawControl( element, option, painter, widget );
        }
        break;
    default:

        QProxyStyle::drawControl( element, option, painter, widget );
    }
}
