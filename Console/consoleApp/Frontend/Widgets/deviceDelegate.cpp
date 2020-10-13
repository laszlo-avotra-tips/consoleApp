#include "deviceDelegate.h"
#include "logger.h"
#include <QPainter>
#include <QBrush>

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected){
        const auto& rect = option.rect;
        painter->fillRect(rect, QBrush(Qt::gray));
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
//    return QStyledItemDelegate::sizeHint(option, index); //rezults in packet items
}
