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
//    const int x0 = 85;
//    const int y0 = 50;
//    const int yImage = y0 + 250 * index.row();

    QVariant vImage = index.data();

    painter->save();

    if(vImage.isValid()){
        if(vImage.toString().isEmpty()){
            QImage image = vImage.value<QImage>();
            QImage iHW = image.scaled(160,160);
            painter->drawImage(0,0,iHW);
        }
    }
    QStyledItemDelegate::paint(painter, option, index);

    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(750,250); //QStyledItemDelegate::sizeHint(option, index);
}
