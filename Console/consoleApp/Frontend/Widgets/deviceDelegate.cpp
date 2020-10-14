#include "deviceDelegate.h"
#include "logger.h"
#include <QPainter>
#include <QBrush>
#include <QPalette>
#include <QColor>

DeviceDelegate::DeviceDelegate(QObject *parent) : QItemDelegate(parent)
{    

}

void DeviceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem myOption(option);
    if (option.state & QStyle::State_Selected){
        QPalette myPalette(option.palette);
//        myPalette.setBrush(QPalette::Highlight,QBrush(QColor(93,93,93)));
        myPalette.setBrush(QPalette::Highlight,Qt::black);
        myOption.palette = myPalette;
    }

    QItemDelegate::paint(painter, myOption, index);
    painter->restore();
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
}

void DeviceDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    if (option.state & QStyle::State_Selected){
        QRect myRect(rect.bottomRight(),rect.bottomRight());
        QItemDelegate::drawDisplay(painter,option,myRect,"");
    } else {
        QItemDelegate::drawDisplay(painter,option,rect,text);
    }
}

void DeviceDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
    if (option.state & QStyle::State_Selected){
        QSize mySize(861,250);
        QIcon myIcon(":/octConsole/measureYellow");
        QPixmap myPixmap(myIcon.pixmap(mySize));

//        myPixmap.fill(QColor(93,93,93));
        QPoint iconTopLeft = rect.topLeft();
        QPoint origin(iconTopLeft.x()- 5, iconTopLeft.y() - 60);
        QRect myRect(origin,mySize);
        QItemDelegate::drawDecoration(painter,option,myRect,myPixmap);
    } else {
        QItemDelegate::drawDecoration(painter,option,rect,pixmap);
    }
}
