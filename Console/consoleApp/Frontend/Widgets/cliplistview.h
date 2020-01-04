/*
 * clipListview.h
 *
 * An overloaded version of the listview Qt class providing custom
 * tooltips for the captures.
 *
 * Author: Chris White, Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#ifndef CLIPLISTVIEW_H
#define CLIPLISTVIEW_H

#include <QListView>

class clipListView : public QListView
{
Q_OBJECT

public:
    explicit clipListView(QWidget *parent = nullptr);

signals:

public slots:
    void updateView( const QModelIndex &, int, int );

protected:
    bool viewportEvent(QEvent *event);

private:
};

#endif // CLIPLISTVIEW_H
