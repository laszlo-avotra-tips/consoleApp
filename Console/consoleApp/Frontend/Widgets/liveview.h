/*
 * liveview.h
 *
 * Implements the view for the live OCT images (sector and waterfall).
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */

#pragma once

#include "buildflags.h"
#include <QGraphicsView>

class liveView : public QGraphicsView
{
    Q_OBJECT

public:
    liveView(QWidget *parent = 0);
    ~liveView();

signals:

private:

protected:
protected slots:
};
