/*
 * auxmonitor.h
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2016-2017 Avinger, Inc.
 */
#ifndef AUXMONITOR_H
#define AUXMONITOR_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QString>
#include <QMatrix>
#include <QTransform>
#include <QResizeEvent>
#include <QScrollBar>
#include "windowmanager.h"
#include "ui_auxmonitor.h"

class AuxMonitor : public QMainWindow
{
    Q_OBJECT

public:

    enum LabelEnum
    {
        CatheterView,
        Recording,
        ZoomFactor,
        TimeField,
        Status
    };

    struct T_Label
    {
        LabelEnum labelName;
        QLabel    *uiLabel;
        QString   cachedText;
    };

    explicit AuxMonitor(QWidget *parent = 0);
    void setScene(QGraphicsScene *scene);
    ~AuxMonitor();

    void updateTime( QString sTime );
    void setDeviceName( QString sDevice );
    QMatrix getMatrix( void );
    void setTransformForZoom( QTransform t, bool isZoomed );
    void configureDisplayForReview( void );
    void configureDisplayForLiveView( void );
    void setText( LabelEnum labelName, bool showText, QString text = "" );
    void liveGraphicsViewCenterOn( qreal x, qreal y );
    const QScrollBar *getLiveGraphicsViewHorizontalScrollBar();
    const QScrollBar *getLiveGraphicsViewVerticalScrollBar();
    void setLiveGraphicsViewHorizontalScrollBar( int val );
    void setLiveGraphicsViewVerticalScrollBar( int val );

public slots:
    void forceResize();

private:
    Ui::AuxMonitorClass ui;
    WindowManager *wm;
    void optimizeSceneSize();
    void testAndMoveInfo();
    T_Label catheterViewLabel;
    T_Label recordingLabel;
    T_Label zoomFactorLabel;
    T_Label timeFieldLabel;
    T_Label statusLabel;

protected:
    void resizeEvent( QResizeEvent *event );
};

#endif // AUXMONITOR_H
