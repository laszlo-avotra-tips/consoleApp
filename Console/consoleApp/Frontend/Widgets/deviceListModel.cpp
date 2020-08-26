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
        return m_data[index.row()].m_name1;
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
            DeviceModel dm(d->getDeviceName());
            m_data.append(dm);
        }
    }
    endResetModel();
}
