#include "rotationIndicatorOverlay.h"
#include <QColor>
#include <QFont>

RotationIndicatorOverlay::RotationIndicatorOverlay(QGraphicsItem *parent)
    : QGraphicsTextItem( parent )
{
    setDefaultTextColor(QColor("red"));
    setFont(QFont("Helvetica Neue"));
}

void RotationIndicatorOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
    setPlainText("Active");
}
