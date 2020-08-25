#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractItemModel>

class DeviceListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DeviceListModel(QObject * parent = 0);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void populate();

};

#endif // DEVICELISTMODEL_H
