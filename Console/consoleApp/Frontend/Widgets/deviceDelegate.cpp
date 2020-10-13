#include "deviceDelegate.h"
#include "logger.h"
#include <QPainter>
#include <QBrush>
#include <QPalette>

DeviceDelegate::DeviceDelegate(QObject *parent) : QItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem myOption(option);
//    if (option.state & QStyle::State_Selected){
//        const auto& rect = option.rect;
//        const auto& color = option.palette.highlight(); // QBrush(Qt::gray)
        QPalette myPalette(option.palette);
        myPalette.setBrush(QPalette::Highlight,QBrush(Qt::gray));
        myPalette.setBrush(QPalette::HighlightedText,QBrush(Qt::yellow));
        myOption.palette = myPalette;
//        painter->fillRect(rect, color);
//    }

    QItemDelegate::paint(painter, myOption, index);
    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
//    return QStyledItemDelegate::sizeHint(option, index); //results in packet items
}
