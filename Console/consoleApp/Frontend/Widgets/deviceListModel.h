#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>

#include "deviceDisplayModel.h"

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    DeviceListModel(QObject * parent = 0);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void populate(bool isCto = false);

    int selectedDeviceIndex() const;
    void setSelectedDeviceIndex(int selectedDeviceIndex);

private:
    QList<DeviceDisplayModel> m_data;
    int m_selectedDeviceIndex{-1};
};

#endif // DEVICELISTMODEL_H
