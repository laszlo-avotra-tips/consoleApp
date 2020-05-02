/*
 * liveview.h
 *
 * Implements the view for the live OCT image (sector).
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
    liveView(QWidget *parent = nullptr);
    ~liveView();

signals:

private:

protected:
protected slots:
};
