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
    painter->save();
    QStyleOptionViewItem myOption(option);
    if (option.state & QStyle::State_Selected){
        QPalette myPalette(option.palette);
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
}

void DeviceDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
    QRect myRect;
//    myRect.setRect(rect.x(), rect.y(), 500, 125);
    myRect.setRect(rect.x() + 100, rect.y(), rect.width(), rect.height());
    if (option.state & QStyle::State_Selected){
        deviceSettings &devices = deviceSettings::Instance();
        auto* image = devices.getSelectedIcon();
        if(image)
        {
            QPixmap qpm;
            qpm.convertFromImage(*image);
            QItemDelegate::drawDecoration(painter,option,myRect,qpm);
        }
    } else {
        QItemDelegate::drawDecoration(painter,option,myRect,pixmap);
    }
}
