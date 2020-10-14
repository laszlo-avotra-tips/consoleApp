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

//    if ( role == Qt::DisplayRole)
//    {
//        int i = index.row();
//        const DeviceDisplayModel& item = m_data.at(i);

//        QVariant retVal;
//        retVal.setValue(item.name());
//        return retVal;
//    }
    if(role == Qt::DecorationRole){
        int i = index.row();
        const DeviceDisplayModel& item = m_data.at(i);

        QVariant retVal;
        retVal.setValue(item.image());
        return retVal;
    }
//    if(role == Qt::ForegroundRole){
//        QVariant retVal;
//        retVal.setValue(QBrush(Qt::white));
//        return retVal;
//    }

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
                DeviceDisplayModel dm(d->getSplitDeviceName(), d->getIcon());
                m_data.append(dm);
            }
            if(!d->isAth() && isCto){
                DeviceDisplayModel dm(d->getSplitDeviceName(), d->getIcon());
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
