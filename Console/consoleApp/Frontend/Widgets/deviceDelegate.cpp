#include "deviceDelegate.h"
#include "deviceSettings.h"
#include "deviceDisplayModel.h"
#include "deviceListModel.h"
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

    const int yText = 200 * index.row() + 20;
    const int yImage = 200 * index.row();

    painter->drawImage(20,yImage,ddm.image());
    painter->drawText(200, yText, ddm.name());

   QStyledItemDelegate::paint(painter, option, index);

    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(400,200); //QStyledItemDelegate::sizeHint(option, index);
}
