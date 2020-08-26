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
        return m_data[index.row()].name();
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
            DeviceDisplayModel dm(d->getDeviceName(), d->getIcon());
            m_data.append(dm);
        }
    }
    endResetModel();
}
