#include "deviceDelegate.h"
#include "deviceSettings.h"
#include "deviceDisplayModel.h"
#include "deviceListModel.h"
#include "logger.h"

#include <QPainter>
#include <QImage>
#include <QStringList>
#include <QApplication>

DeviceDelegate::DeviceDelegate(QObject *parent) : QStyledItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(true)
    {
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
    } else {

        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);

        LOG1(index.row())

//        if ((itemOption.state & QStyle::State_Selected) &&
//            (itemOption.state & QStyle::State_Active))
        {
            itemOption.palette.setColor(QPalette::Highlight, Qt::red);  // set your color here
        }

        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, nullptr);

    }

}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(750,250); //QStyledItemDelegate::sizeHint(option, index);
}
