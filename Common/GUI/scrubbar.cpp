/*
 * scrubbar.cpp
 *
 * Author: Chris White
 *
 * Copyright (c) 2015-2018 Avinger, Inc.
 */
#include "scrubbar.h"

scrubbar::scrubbar(QWidget *parent) :
    QSlider(parent)
{
    setTracking( false );
}
