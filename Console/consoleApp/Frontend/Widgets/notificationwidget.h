/*
 * notificationwidget.h
 *
 * The notification widget is a simple animated widget that provides
 * a fade in/out message on the screen with the desired text.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger
 */

#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <QTimeLine>
#include <QPainter>

class notificationWidget : public QWidget
{
Q_OBJECT
Q_PROPERTY( QBrush fadeBrush READ fadeBrush WRITE setFadeBrush )
Q_PROPERTY( int fadeDuraction READ fadeDuration WRITE setFadeDuration )
public:
    explicit notificationWidget(QWidget *parent = nullptr);
    QBrush fadeBrush() const { return startBrush; }
    void setFadeBrush(const QBrush &newColor) { startBrush = newColor; }

    int fadeDuration() const { return timeLine->duration(); }
    void setFadeDuration(int milliseconds) { timeLine->setDuration(milliseconds); }

    void start();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QTimeLine *timeLine;
    QBrush startBrush;
};

#endif // NOTIFICATIONWIDGET_H
