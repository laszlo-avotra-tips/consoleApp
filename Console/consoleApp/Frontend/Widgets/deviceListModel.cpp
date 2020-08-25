#include "deviceListModel.h"
#include "deviceSettings.h"

DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{

}

int DeviceListModel::rowCount(const QModelIndex &) const
{
    return m_data.size();
}

int DeviceListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ( role == Qt::DisplayRole)
    {
        if ( index.column() == 0)
            return m_data[index.row()].m_name1;

        if ( index.column() == 1)
            return m_data[index.row()].m_isAth;
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
            DeviceModel dm(d->getDeviceName(), d->isAth());
            m_data.append(dm);
        }
    }
    endResetModel();
}
