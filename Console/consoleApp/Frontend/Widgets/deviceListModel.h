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

private:
    QList<DeviceDisplayModel> m_data;
};

#endif // DEVICELISTMODEL_H
