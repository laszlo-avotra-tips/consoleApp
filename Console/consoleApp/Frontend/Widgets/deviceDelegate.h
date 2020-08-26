#ifndef DEVICEDELEGATE_H
#define DEVICEDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class DeviceDelegate : public QStyledItemDelegate
{
public:
    DeviceDelegate(QObject * parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // DEVICEDELEGATE_H
