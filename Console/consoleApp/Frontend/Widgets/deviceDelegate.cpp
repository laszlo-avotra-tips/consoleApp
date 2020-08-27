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
//    const auto* amodel = index.model();
//    auto model = dynamic_cast<const DeviceListModel*>(amodel);
//    if(model){

//        int i = index.row();
//        const auto& mdata = model->data(index,Qt::DisplayRole);
//        const auto& myType = mdata.value<DeviceDisplayModel>();
//        LOG2(i,myType.name())
//    }
    {
        LOG2(ddm.name(),index.row())
    }

//    painter->drawImage(0,20,ddm.image());
    painter->drawText(20,20,ddm.name());
    QStyledItemDelegate::paint(painter, option, index);

    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(200,100); //QStyledItemDelegate::sizeHint(option, index);
}
