#ifndef DEVICEDELEGATE_H
#define DEVICEDELEGATE_H

#include <QObject>
#include <QItemDelegate>

class DeviceDelegate : public QItemDelegate
{
public:
    DeviceDelegate(QObject * parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;
};

#endif // DEVICEDELEGATE_H
