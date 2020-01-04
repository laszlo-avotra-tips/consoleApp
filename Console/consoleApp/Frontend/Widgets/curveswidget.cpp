/*
 * curveswidget.cpp
 *
 * non-linear grayscale adjustment UI
 *
 * Author: Chris White
 * Copyright (c) 2009-2018 Avinger, Inc.
 */
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include "curveswidget.h"

curvesWidget::curvesWidget(QWidget *parent)
    : QWidget(parent)
{
    mouseDown = false;
    curveMap.resize(256);
    reset();
    controlPoint1.setX(0.0);
    controlPoint1.setY(0.0);
}

curvesWidget::~curvesWidget()
{

}

void curvesWidget::reset(void)
{
    ushort i;

    // Set up a straight linear map
    for (i = 0; i <= maxLevel; i++)
    {
        curveMap[i] = uchar(i);
    }
    emit notifyCurveChanged();
}

void curvesWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    QPolygonF curve;

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QBrush(QColor(0,0,0), Qt::SolidPattern));
    painter.setPen(QPen(QColor(0,0,0), Qt::SolidPattern));
    painter.drawRect(0, 0, 255, 255);

    painter.setPen(QPen(QColor(0, 255, 0), 1, Qt::SolidLine, Qt::RoundCap)); // magic XXX
    painter.setBrush(QBrush(QColor(0, 255, 0), Qt::SolidPattern));

    for (int i = 0; i <= maxLevel; i++) {
        curve << QPointF(i, 255.0 - curveMap[i]);
    }
    painter.drawPolyline(curve);
    painter.end();
}

void curvesWidget::mousePressEvent(QMouseEvent *event)
{
    if (mouseDown) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        mouseDown = true;
        controlPoint2.setX(event->pos().x());
        controlPoint2.setY(event->pos().y());
        recalcCurve();
    }
}

void curvesWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!mouseDown) {
        return;
    }   
    
    controlPoint2.setX(event->pos().x());
    controlPoint2.setY(event->pos().y());
    recalcCurve();
}

void curvesWidget::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    mouseDown = false;
}

void curvesWidget::mouseDoubleClickEvent(QMouseEvent * /*event*/)
{
    mouseDown = false;
    reset();
    update();
}

void curvesWidget::recalcCurve(void)
{
    short i;
    double pcnt;

    QPainterPath path;

    // Do a simple cubic interpolation, but make qt do it. I'm lazy.
    path.moveTo(0.0, 255.0);
    controlPoint1 = controlPoint2;
    controlPoint1.setY(controlPoint1.y() - 20);
    path.cubicTo(controlPoint1, controlPoint2, QPointF(255.0, 0.0));
   
    for (i = 0; i <= maxLevel; i++) {
        pcnt = double(i) / 255.0;
        if (path.pointAtPercent(pcnt).y() > 255) {
            curveMap[i] = 0;
        } else if (path.pointAtPercent(pcnt).y() < 0) {
            curveMap[i] = 255;
        } else {
            curveMap[i] = uchar(255.0 - path.pointAtPercent(pcnt).y());
        }
    }
    emit notifyCurveChanged();
    update();
}

QVector<unsigned char> curvesWidget::getMap(void)
{
    return(curveMap);
}
