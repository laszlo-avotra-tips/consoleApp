#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>

#include "deviceDisplayModel.h"

//struct DeviceDisplayModel{
//    DeviceDisplayModel(const QString& name)
//        : m_name(name)
//    {}

//    QString m_name;
//};

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    DeviceListModel(QObject * parent = 0);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void populate();

private:
    QList<DeviceDisplayModel> m_data;
};

#endif // DEVICELISTMODEL_H
