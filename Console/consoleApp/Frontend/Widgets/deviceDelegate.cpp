#include "deviceDelegate.h"
#include "deviceSettings.h"
#include "deviceDisplayModel.h"
#include "logger.h"

#include <QPainter>
#include <QImage>

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QVariant vImage = index.data();

    DeviceDisplayModel ddm = vImage.value<DeviceDisplayModel>();
    LOG2(ddm.name(),index.row())

//    painter->drawImage(0,20,ddm.image());
//    painter->drawText(200,20,ddm.name());
    QStyledItemDelegate::paint(painter, option, index);

    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(200,100); //QStyledItemDelegate::sizeHint(option, index);
}
