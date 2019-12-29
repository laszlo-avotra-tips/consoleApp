/*
 * backgroundmask.cpp
 *
 * Displays a black and 75% opaque label over the parent widget.
 * Create an instance of backgroundMask in a widget or dialog when
 * dimming the background calling widget/window is desired to bring
 * the widget to attention and create contrast from the background.
 *
 * Author: Ryan Radjabi
 * Copyright (c) 2016-2018 Avinger, Inc.
 */
#include "backgroundmask.h"
#include <QDebug>

/*
 * constructor
 */
backgroundMask::backgroundMask(QWidget *parent) :
    QWidget(parent)
{
    mask = NULL;

    /*
     * Check for NULL parent. This could happen when backgroundMask is created
     * from a widget without a parent widget.
     */
    if( parent != NULL )
    {
        init( parent->topLevelWidget() );
    }
}

/*
 * destructor
 */
backgroundMask::~backgroundMask()
{
    if( mask != NULL )
    {
        delete mask;
        mask = NULL;
    }
}

/*
 * init
 *
 * Create a black and 75% opaque label over the parent widget.
 */
void backgroundMask::init( QWidget *parent )
{
    mask = new QLabel( parent );
    mask->setStyleSheet( "QLabel { background-color: rgba( 0, 0, 0, 75% ) }" );
    mask->setGeometry( parent->rect() );
    mask->stackUnder( parent );
    mask->show();
}
