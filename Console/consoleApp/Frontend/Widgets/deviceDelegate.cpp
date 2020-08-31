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
    LOG1(index.row())

    const int x0 = 85;
    const int y0 = 50;
    const int yText = 2 * y0 + 250 * index.row() ;
    const int yImage = y0 + 250 * index.row();

    QVariant vImage = index.data();

    painter->save();

    if(vImage.isValid()){
        if(!vImage.toString().isEmpty()){
            QString text = vImage.toString();
            QStringList devName = text.split("\n");
//            painter->drawText(x0 + 250, yText, devName[0]);
//            painter->drawText(x0 + 250, yText + 80, devName[1]);
        } else {

            QImage image = vImage.value<QImage>();
            painter->drawImage(x0,yImage,image);
        }

    }
//    DeviceDisplayModel ddm = vImage.value<DeviceDisplayModel>();
//    painter->drawImage(x0,yImage,ddm.image());
    QStyledItemDelegate::paint(painter, option, index);

    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(750,250); //QStyledItemDelegate::sizeHint(option, index);
}
