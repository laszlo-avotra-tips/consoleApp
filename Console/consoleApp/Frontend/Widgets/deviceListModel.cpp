#include "deviceListModel.h"
#include "deviceSettings.h"

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
        retVal.setValue<DeviceDisplayModel>(item);
        return retVal;
    }

    return QVariant();
}

void DeviceListModel::populate()
{
    beginResetModel();
    m_data.clear();
    {
        deviceSettings &devices = deviceSettings::Instance();
        QList<device *>devList = devices.list();
        for ( device* d : devList )
        {
            DeviceDisplayModel dm(d->getSplitDeviceName(), d->getIcon());
            m_data.append(dm);
        }
    }
    endResetModel();
}
