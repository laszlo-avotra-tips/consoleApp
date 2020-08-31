#include "deviceDelegate.h"
#include "deviceSettings.h"
#include "deviceDisplayModel.h"
#include "deviceListModel.h"
#include "logger.h"

#include <QPainter>
#include <QImage>
#include <QStringList>

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QVariant vImage = index.data();

    DeviceDisplayModel ddm = vImage.value<DeviceDisplayModel>();

    const int x0 = 85;
    const int y0 = 50;
    const int yText = 2 * y0 + 250 * index.row() ;
    const int yImage = y0 + 250 * index.row();

    QString text{ddm.name()};
    QStringList devName = text.split("\n");

    painter->drawImage(x0,yImage,ddm.image());
    painter->drawText(x0 + 250, yText, devName[0]);
    painter->drawText(x0 + 250, yText + 80, devName[1]);

   QStyledItemDelegate::paint(painter, option, index);

    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(750,250); //QStyledItemDelegate::sizeHint(option, index);
}
