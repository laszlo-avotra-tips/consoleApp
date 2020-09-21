#include "rotationIndicatorOverlay.h"
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPointF>

RotationIndicatorOverlay::RotationIndicatorOverlay(QGraphicsItem *parent)
    : QGraphicsTextItem( parent )
{
    setTextWidth(400);
    auto font = QFont("Helvetica Neue");
    setFont(font);
}

void RotationIndicatorOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
    QFont font = painter->font();
    font.setPointSize(font.pointSize() * 3 );
    painter->setFont(font);
    painter->setPen( QPen( QBrush( QColor( 255, 215, 0 ), Qt::SolidPattern ), 2 ) );
    painter->drawText(QPointF(400,530), "Passive"); // " Active"
}
