#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>
#include <QString>

struct DeviceModel{
    DeviceModel(const QString& name1)
        : m_name1(name1)
    {}

    QString m_name1;
};

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    DeviceListModel(QObject * parent = 0);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void populate();

private:
    QList<DeviceModel> m_data;
};

#endif // DEVICELISTMODEL_H
