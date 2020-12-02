/*
 * clipListview.cpp
 *
 * An overloaded version of the listview Qt class providing custom
 * tooltips for the captures.
 *
 * Author: Chris White, Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#include "cliplistview.h"
#include <QEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <QPainter>

clipListView::clipListView(QWidget *parent) :
    QListView(parent)
{
    QPalette pal;
    setMouseTracking(true);
    pal.setColor( QPalette::Inactive, QPalette::ToolTipBase, Qt::darkGray );
    pal.setColor( QPalette::Inactive, QPalette::ToolTipText, Qt::white );

    QToolTip::setPalette( pal );
    QToolTip::setFont( QFont( "DINPro-Medium", 16 ) );
    setFlow(QListView::Flow::LeftToRight);
}

/*
 * viewPortEvent()
 *
 * In order to fully customize tooltips, we look at all
 * viewport events for the list and if it is of type
 * ToolTip, we display our informational popup.
 */
bool clipListView::viewportEvent( QEvent *event )
{
    return( QListView::viewportEvent(event) );
}

/*
 * updateView()
 *
 * Slot to allow a signal from the the database to update the view.
 */
void clipListView::updateView( const QModelIndex &/*topLeft*/, int /*start*/, int /*end*/)
{
    this->scrollToBottom();
}
