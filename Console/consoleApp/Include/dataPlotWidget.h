///*
// * dataPlotWidget.cpp
// *
// * Declarations for a widget plot, derived from the basic qwt plot object,
// * which can display data in real-time. Changes from basic qwt include
// * deferred, timer-based rendering, and a default set of curves.
// *
// * Author: Chris White
// *
// * Copyright (c) 2010-2018 Avinger Inc.
// */
//#pragma once

//#include <qwt_plot.h>
//#include <QTimer>
//#include <qwt_plot_curve.h>
//#include <qwt_picker.h>
//#include <QPen>
//#include <qwt_plot_grid.h>
//#include <qwt_scale_draw.h>
//#include <qwt_plot_marker.h>
//#include "buildflags.h"

//const int updateTimeMS(33); //lcv

//class dataPlotWidget : public QwtPlot, public QwtScaleDraw
//{
//    Q_OBJECT

//public:
//    dataPlotWidget( QWidget * = nullptr );
//    ~dataPlotWidget();
//    void init ( int size = 2048 );
//    void plotData( unsigned short * );
//    void plotData( unsigned char * );
//    void enableDisplayControls() { displayControlsEnabled = true; }
//    void disableDisplayControls() { displayControlsEnabled = false; }
//#if CONSOLE_MANUFACTURING_RELEASE
//    void drawMmDepthLines( void );
//    void drawInternalMaskLine( int iMask_px );
//#endif

//signals:
//    void updateBrightness(int);
//    void updateContrast(int);

//public slots:
//    void setYAxisMax( double val ) { setAxisScale( QwtPlot::yLeft, 0, val ); };
//    void handleUpdate();
//    void enableLevels();
//    void changeBrightness(int);
//    void changeContrast(int);
//    void clearData();
//    void setBrightnessLimits( int lower, int upper );
//    void setContrastLimits(int lower, int upper);
//    void setMousePressTolerance( int val ) { mousePressTolerance = val; }

//protected:
//    void mousePressEvent( QMouseEvent *event);
//    void mouseReleaseEvent( QMouseEvent *event );
//    void mouseMoveEvent( QMouseEvent *event );

//private:
//    int plotSize;
//    QwtPicker *picker;
//    QTimer updateTimer;
//    QwtPlotCurve *cData;
//    double *d_x;
//    double *d_y;
//    int currContrast;
//    QwtPlotCurve *contrastCurve;
//    int currBrightness;
//    QwtPlotCurve *brightnessCurve;
//    QPen brightnessPen;
//    QPen contrastPen;
//    QwtPlotGrid *grid;
//#if CONSOLE_MANUFACTURING_RELEASE
//    QwtPlotMarker *maskMarker;
//#endif

//    bool showLevels;
//    bool displayControlsEnabled;
//    bool contrastDragging;
//    bool brightnessDragging;

//    int  minBrightness;
//    int  minContrast;

//    int  maxBrightness;
//    int  maxContrast;

//    int mousePressTolerance;

//    void plotLevels();
//    void alignScales();
//};
