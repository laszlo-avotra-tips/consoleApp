/*
 * docscreen.h
 *
 * The secondary window for the OCT application. This window
 * presents the physician with a stripped down view of
 * the GUI, focused on the sector
 *
 * Author: Chris White
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 */
#ifndef DOCSCREEN_H
#define DOCSCREEN_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSortFilterProxyModel>
#include <QString>
#include "ui_docscreen.h"

class docscreen : public QMainWindow
{
    Q_OBJECT

public:
    Ui::docscreenClass ui;

    docscreen(QWidget *parent);
    ~docscreen();
    void setScene(QGraphicsScene *scene);
    void configureDisplayForReview( void );
    void configureDisplayForLiveView( void );

public slots:
    void showPreview( bool );
    void updatePreview( QModelIndex );
    void setDeviceName( QString dev );

private:
     QSortFilterProxyModel *proxyModel;
   
};

#endif // DOCSCREEN_H
