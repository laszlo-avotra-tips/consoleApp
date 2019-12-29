/*
 * curveswidget.h
 *
 * non-linear grayscale adjustment UI
 *
 * Author: Chris White
 * Copyright (c) 2009-2018 Avinger, Inc.
 */
#ifndef CURVESWIDGET_H
#define CURVESWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QVector>

const int maxLevel(255);

class curvesWidget : public QWidget
{
    Q_OBJECT

public:
    curvesWidget(QWidget *parent = 0);
    ~curvesWidget();

    QVector<unsigned char> getMap(void);
    void reset();

signals:
    void notifyCurveChanged(void);
private:
    bool mouseDown;
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void recalcCurve(void);

    QVector<unsigned char> curveMap;
    QPointF controlPoint1;
    QPointF controlPoint2;

};

#endif // CURVESWIDGET_H
