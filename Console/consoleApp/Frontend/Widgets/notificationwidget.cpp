/*
 * notificationwidget.cpp
 *
 * The notification widget is a simple animated widget that provides
 * a fade in/out message on the screen with the desired text.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger
 */

#include "notificationwidget.h"

notificationWidget::notificationWidget(QWidget *parent) :
    QWidget(parent)
{
    if (parent)
        startBrush = parent->palette().window();
    else
        startBrush = Qt::white;
    timeLine = new QTimeLine(1000, this);
    timeLine->setFrameRange(1000, 0);
    connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(update()));

    setAttribute(Qt::WA_DeleteOnClose);
    resize(parent->size());
}

void notificationWidget::start()
{
    timeLine->start();
    show();
}

void notificationWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    qreal frame = timeLine->currentFrame();
    if ( frame > 500 )
        painter.setOpacity( ( 1000 - frame ) / 500 );
    else
        painter.setOpacity( ( frame ) / 500 );
    painter.fillRect(rect(), startBrush);
    if (frame <= 0)
        close();
}
