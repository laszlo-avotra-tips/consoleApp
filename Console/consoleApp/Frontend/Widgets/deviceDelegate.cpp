#include "deviceDelegate.h"
#include "deviceSettings.h"
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
//    painter->save();
//    QStyleOptionViewItem myOption(option);
//    if (option.state & QStyle::State_Selected){
//        QPalette myPalette(option.palette);
////        myPalette.setBrush(QPalette::Highlight,QBrush(QColor(93,93,93)));
//        myPalette.setBrush(QPalette::Highlight,Qt::black);
//        myOption.palette = myPalette;
//    }

//    QItemDelegate::paint(painter, myOption, index);
//    painter->restore();
    QItemDelegate::paint(painter, option, index);
}

QSize DeviceDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return QSize(750,250);
}

void DeviceDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
//    if (option.state & QStyle::State_Selected){
//        QRect myRect(rect.bottomRight(),rect.bottomRight());
//        QItemDelegate::drawDisplay(painter,option,myRect,"");
//    } else {
//        QItemDelegate::drawDisplay(painter,option,rect,text);
//    }
}

void DeviceDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
//    if (option.state & QStyle::State_Selected){
//        QSize mySize(861,250);
//        QIcon myIcon(":/octConsole/measureYellow");
//        QPixmap myPixmap(myIcon.pixmap(mySize));

//        QPoint iconTopLeft = rect.topLeft();
//        QPoint origin(iconTopLeft.x(), iconTopLeft.y() - 60);
//        QRect myRect(origin,mySize);
//        QItemDelegate::drawDecoration(painter,option,myRect,myPixmap);
//    } else {
    if (option.state & QStyle::State_Selected){
        deviceSettings &devices = deviceSettings::Instance();
        auto* dev = devices.current();
        LOG1(dev)
        if(dev){
            QImage* highlightImage = dev->getIcon()[0];
            QPixmap qpm;
            if(highlightImage){
                qpm.convertFromImage(*highlightImage);
                QItemDelegate::drawDecoration(painter,option,rect,qpm);
            }
        }
//        QIcon myIcon(":/octConsole/measureYellow");
//        QPixmap myPixmap(myIcon.pixmap(rect.size()));
//        QItemDelegate::drawDecoration(painter,option,rect,myPixmap);
    } else {
        QItemDelegate::drawDecoration(painter,option,rect,pixmap);
    }
}
