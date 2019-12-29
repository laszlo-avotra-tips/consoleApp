/*
 * liveview.cpp
 *
 * Implements the view for the live OCT images (sector and waterfall).
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#include <QDebug>
#include <QMouseEvent>
#include "liveview.h"

liveView::liveView(QWidget *parent)
    : QGraphicsView(parent)
{

}

liveView::~liveView()
{

}
