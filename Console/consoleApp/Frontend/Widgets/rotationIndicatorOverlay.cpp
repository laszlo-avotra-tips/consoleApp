#include "rotationIndicatorOverlay.h"
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPointF>

RotationIndicatorOverlay::RotationIndicatorOverlay(QGraphicsItem *parent)
    : QGraphicsTextItem( parent )
{
    setDefaultTextColor(QColor("red"));
    setFont(QFont("Helvetica Neue"));
}

void RotationIndicatorOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
//    setPlainText("Active");
    painter->setPen( QPen( QBrush( QColor( 255, 215, 0 ), Qt::SolidPattern ), 2 ) );
    painter->drawText(QPointF(512,512), "Active");
}
