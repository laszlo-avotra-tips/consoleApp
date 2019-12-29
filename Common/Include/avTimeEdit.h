/*
 * avTimeEdit.h
 *
 * Interface for a custom QTimeEdit box that indicates when the user has made
 * changes to it.  The standard QTimeEdit object emits a timeChanged()
 * signal. This implementation allows the QTimeEdit box to constantly
 * update to the current time until the user makes a change.  This
 * behavior is similar to how Windows handles time changes.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2010-2017 Avinger, Inc.
 *
 */
#ifndef AVTIMEEDIT_H
#define AVTIMEEDIT_H

#include <QKeyEvent>
#include <QTimeEdit>

class avTimeEdit : public QTimeEdit
{
Q_OBJECT
public:
    explicit avTimeEdit(QWidget *parent = 0);

signals:
    void userUpdate();

protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);

public slots:

private:

};

#endif // AVTIMEEDIT_H
