/*
 * avTimeEdit.cpp
 *
 * A custom QTimeEdit box that indicates when the user has made
 * changes to it.  The standard QTimeEdit object emits a timeChanged()
 * signal. This implementation allows the QTimeEdit box to constantly
 * update to the current time until the user makes a change.  This
 * behavior is similar to how Windows handles time changes.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */

#include "avTimeEdit.h"

/*
 * Constructor
 *
 */
avTimeEdit::avTimeEdit(QWidget *parent) :
    QTimeEdit(parent)
{
}

/*
 * keyPressEvent
 *
 * Only allow defined keys to stop the clock in the time edit box to signal
 * that the user is changing the time.  The key is sent to the parent object
 * in order to use it.
 */
void avTimeEdit::keyPressEvent(QKeyEvent *event)
{
    const int Key = event->key();
    if( ( Key == Qt::Key_Up   ) ||
        ( Key == Qt::Key_Down ) ||
        ( Key == Qt::Key_Escape ))
    {
        emit userUpdate();

        // pass the event along to QTimeEdit
        QTimeEdit::keyPressEvent(event);
    }
}

/*
 * mousePressEvent
 *
 * If any mouse button is used in the time edit box, signal that the
 * user is changing the time.  The button press is sent to the parent object
 * in order to use it.
 */
void avTimeEdit::mousePressEvent(QMouseEvent *event)
{
    emit userUpdate();

    // pass the event along to QTimeEdit
    QTimeEdit::mousePressEvent(event);
}
