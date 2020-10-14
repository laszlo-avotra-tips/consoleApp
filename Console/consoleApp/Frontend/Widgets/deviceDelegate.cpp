#include "deviceDelegate.h"
#include "logger.h"
#include <QPainter>
#include <QBrush>
#include <QPalette>
#include <QColor>

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem myOption(option);
    if (option.state & QStyle::State_Selected){
        QPalette myPalette(option.palette);
        myPalette.setBrush(QPalette::Highlight,QBrush(QColor(93,93,93)));
        myOption.palette = myPalette;
    }

    QStyledItemDelegate::paint(painter, myOption, index);
    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
}
