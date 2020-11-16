/*
 * capturelistview.h
 *
 * An overloaded version of the listview Qt class providing custom
 * tooltips for the captures.
 *
 * Author: Chris White
 *
 * Copyright 2010-2018 Avinger
 */
#ifndef CAPTURELISTVIEW_H
#define CAPTURELISTVIEW_H

#include <QListView>

class captureListView : public QListView
{
Q_OBJECT
public:
    explicit captureListView(QWidget *parent = 0);

signals:

public slots:
    void updateView( const QModelIndex &, int, int );

protected:
    bool viewportEvent(QEvent *event);
private:
};

#endif // CAPTURELISTVIEW_H
