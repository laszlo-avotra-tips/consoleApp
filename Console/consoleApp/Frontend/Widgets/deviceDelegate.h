#ifndef DEVICEDELEGATE_H
#define DEVICEDELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QPoint>

class DeviceDelegate : public QItemDelegate
{
public:
    DeviceDelegate(QObject * parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const override;

    void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const override;
};

#endif // DEVICEDELEGATE_H
