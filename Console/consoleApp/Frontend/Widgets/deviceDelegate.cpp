#include "deviceDelegate.h"
#include "logger.h"
#include <QPainter>
#include <QBrush>
#include <QPalette>

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption(option);
    if (option.state & QStyle::State_Selected){
//        const auto& rect = option.rect;
//        const auto& color = option.palette.highlight(); // QBrush(Qt::gray)
        QPalette pal(option.palette);
//        pal.setBrush(QPalette::ColorRole::Button,QBrush(Qt::gray));
//        pal.setBrush(QPalette::ColorRole::ButtonText,QBrush(Qt::white));
        myOption.palette = pal;
//        painter->fillRect(rect, color);
    }

    QStyledItemDelegate::paint(painter, myOption, index);
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
//    return QStyledItemDelegate::sizeHint(option, index); //rezults in packet items
}
