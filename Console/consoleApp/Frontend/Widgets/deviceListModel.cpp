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
                DeviceDisplayModel dm(d->getDeviceName(), d->getIcon());
                m_data.append(dm);
            }
            if(!d->isAth() && isCto){
                DeviceDisplayModel dm(d->getDeviceName(), d->getIcon());
                m_data.append(dm);
            }
        }
    }
    endResetModel();
}
