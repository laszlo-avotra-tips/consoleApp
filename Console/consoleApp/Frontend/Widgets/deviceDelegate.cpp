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

    const int x0 = 85;
    const int y0 = 80;
    const int yText = 2* y0 + 250 * index.row() ;
    const int yImage = y0 + 250 * index.row();

    painter->drawImage(x0,yImage,ddm.image());
    painter->drawText(x0 + 250, yText, ddm.name());

   QStyledItemDelegate::paint(painter, option, index);

    painter->restore();

    LOG1(ddm.name());
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(750,250); //QStyledItemDelegate::sizeHint(option, index);
}
