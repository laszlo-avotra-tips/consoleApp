#ifndef ROTATIONINDICATOROVERLAY_H
#define ROTATIONINDICATOROVERLAY_H

#include <QGraphicsTextItem>


class RotationIndicatorOverlay : public QGraphicsTextItem
{
public:
    RotationIndicatorOverlay(QGraphicsItem *parent = nullptr);

private:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
};

#endif // ROTATIONINDICATOROVERLAY_H
