#include "deviceListModel.h"
#include "deviceSettings.h"
#include "logger.h"

DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{

}

int DeviceListModel::rowCount(const QModelIndex &) const
{
    return m_data.size();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ( role == Qt::DisplayRole)
    {
        int i = index.row();
        const DeviceDisplayModel& item = m_data.at(i);

        QVariant retVal;
        retVal.setValue(item.name());
        return retVal;
    }
    if(role == Qt::DecorationRole){
        int i = index.row();
        const DeviceDisplayModel& item = m_data.at(i);

        QVariant retVal;
        retVal.setValue(item.image());
        return retVal;
    }
//    if(role == Qt::BackgroundColorRole){
//        QVariant retVal;
//        int i = index.row();
//        if(i == 1){
//            retVal.setValue(QBrush(Qt::white));
//        } else {
//            retVal.setValue(QBrush(Qt::green));
//        }
//        return retVal;
//    }
    if(role == Qt::SizeHintRole){
        QVariant retVal;
        int i = index.row();
        if(i == 1){
            retVal.setValue(QSize(100,50));
        } else {
            retVal.setValue(QSize(400,200));
        }
        return retVal;
    }

    return QVariant();
}

void DeviceListModel::populate(bool isCto)
{
    beginResetModel();
    m_data.clear();
    {
        deviceSettings &devices = deviceSettings::Instance();
        QList<device *>devList = devices.list();
        for ( device* d : devList )
        {
            if(d->isAth() && !isCto){
                DeviceDisplayModel dm(d->getSplitDeviceName(), d->getIcon()[0]);
                m_data.append(dm);
            }
            if(!d->isAth() && isCto){
                DeviceDisplayModel dm(d->getSplitDeviceName(), d->getIcon()[0]);
                m_data.append(dm);
            }
        }
    }
    endResetModel();
}

int DeviceListModel::selectedDeviceIndex() const
{
    return m_selectedDeviceIndex;
}

void DeviceListModel::setSelectedDeviceIndex(int selectedDeviceIndex)
{
    m_selectedDeviceIndex = selectedDeviceIndex;
}
