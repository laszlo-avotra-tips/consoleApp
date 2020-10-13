#include "deviceDelegate.h"
#include "logger.h"

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(option.checkState == Qt::Checked){
        LOG1(option.text)
    }
    QStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
//    return QStyledItemDelegate::sizeHint(option, index); //rezults in packet items
}
