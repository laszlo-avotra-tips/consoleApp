///*
// * dataPlotWidget.cpp
// *
// * Implements a widget plot, derived from the basic qwt plot object,
// * which can display data in real-time. Changes from basic qwt include
// * deferred, timer-based rendering, and a default set of curves.
// *
// * Author: Chris White
// *
// * Copyright (c) 2010-2018 Avinger, Inc.
// */
//#include <QMouseEvent>
//#include <qwt_painter.h>
//#include <qwt_plot_canvas.h>
//#include <qwt_plot_curve.h>
//#include <qwt_scale_widget.h>
//#include <qwt_scale_draw.h>
//#include <qwt_math.h>
//#include <qwt_legend.h>
//#include <QDebug>
//#include <QPointF>
//#include <QPolygonF>
//#include "dataPlotWidget.h"
//#include "defaults.h"
//#include "util.h"

//const QColor CurveColor( 143, 185, 224 );
//const QColor ThresholdColor( 250, 252, 3 );

///*
// * Constructor
// */
//dataPlotWidget::dataPlotWidget( QWidget *parent ):
//    QwtPlot(parent)
//{
//    picker          = nullptr;
//    cData           = nullptr;
//    d_x             = nullptr;
//    d_y             = nullptr;
//    contrastCurve   = nullptr;
//    brightnessCurve = nullptr;

//    showLevels             = false;
//    displayControlsEnabled = false;
//    contrastDragging       = false;
//    brightnessDragging     = false;
//}

///*
// * init
// *
// * Initialize the object and allocate memory for storing data for plotting
// */
//void dataPlotWidget::init( int size )
//{
//    plotSize = size;

//    alignScales();

//    if( d_x )
//    {
//        delete [] d_x;
//    }
//    d_x = new double[ plotSize ];

//    if( d_y  )
//    {
//        delete [] d_y;
//    }
//    d_y = new double[ plotSize ];

//    //  Initialize data
//    for( int i = 0; i < plotSize; i++ )
//    {
//        // fill in initial plot data
//        d_x[ i ] = i;
//        d_y[ i ] = 0;
//    }

//    // Insert new curve
//    if( cData )
//    {
//        delete cData;
//    }
//    cData = new QwtPlotCurve();
//    cData->attach( this );

//    // Set curve style
//    cData->setPen( QPen( CurveColor ) );

//    // Attach (don't copy) data.
//    cData->setRawSamples( d_x, d_y, plotSize );

//    setAutoReplot();
//    connect( &updateTimer, SIGNAL(timeout()), this, SLOT(handleUpdate()) );
//    updateTimer.start( updateTimeMS );

//    showLevels = false;

//    // Default to a Low Speed Device
//    currContrast    = ContrastLevels_LowSpeed.defaultValue;
//    contrastCurve   = nullptr;

//    currBrightness  = BrightnessLevels_LowSpeed.defaultValue;
//    brightnessCurve = nullptr;

//    brightnessDragging = false;
//    contrastDragging   = false;

//    minBrightness = BrightnessLevels_LowSpeed.minValue;
//    minContrast   = ContrastLevels_LowSpeed.minValue;

//    maxBrightness = BrightnessLevels_LowSpeed.maxValue;
//    maxContrast   = ContrastLevels_LowSpeed.maxValue;

//    mousePressTolerance = 2500;

//    contrastPen.setColor( ThresholdColor );
//    contrastPen.setWidth( 2 );
//    brightnessPen.setColor( ThresholdColor.darker( 150 ) );
//    brightnessPen.setWidth( 2 );

//    grid = new QwtPlotGrid();
//    grid->setPen( QPen( Qt::gray, 0.0, Qt::DotLine ) );
//    grid->enableX( false );
//    grid->enableY( true );
//    grid->enableXMin( false );
//    grid->enableYMin( false );
//    grid->attach( this );
//}

///*
// * destructor
// */
//dataPlotWidget::~dataPlotWidget()
//{
//    delete [] d_x;
//    delete [] d_y;
//}

///*
// *  Set a plain canvas frame and align the scales to it.
// *  This is from the original example code.
// */
//void dataPlotWidget::alignScales()
//{
//    // Align the scales to the canvas frame
//    canvas()->setFrameStyle( QFrame::Box | QFrame::Plain );
//    canvas()->setLineWidth( 1 );

//    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
//    {
//        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget( i );
//        if( scaleWidget )
//        {
//            scaleWidget->setMargin( 0 );
//        }

//        QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axisScaleDraw( i );
//        if( scaleDraw )
//        {
//            scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
//        }
//    }
//}

///*
// * plotLevels
// */
//void dataPlotWidget::plotLevels( void )
//{
//    QPolygonF contrastPoly;
//    contrastPoly << QPointF( 0.0, currContrast ) << QPointF( this->plotSize, currContrast );
//    contrastCurve->setSamples( contrastPoly );

//    QPolygonF brightnessPoly;
//    brightnessPoly << QPointF(0.0, currBrightness) << QPointF( this->plotSize, currBrightness );
//    brightnessCurve->setSamples( brightnessPoly );
//}

///*
// * handleUpdate
// */
//void dataPlotWidget::handleUpdate( void )
//{
//    if( isVisible() )
//    {
//        // update the display
//        cData->setRawSamples( d_x, d_y, plotSize );
//    }

//    if( showLevels && contrastCurve && brightnessCurve )
//    {
//        plotLevels();
//    }
//}

///*
// * mousePressEvent
// */
//void dataPlotWidget::mousePressEvent( QMouseEvent *event )
//{
//    if( displayControlsEnabled )
//    {
//        int pnt = invTransform(QwtPlot::yLeft, (event->pos().y()));

//        if( abs(pnt - currContrast) < mousePressTolerance )
//        {
//            contrastDragging = true;
//            currContrast = pnt;
//        }
//        else if( abs(pnt - currBrightness) < mousePressTolerance )
//        {
//            brightnessDragging = true;
//            currBrightness = pnt;
//        }
//        // else ignore the event
//    }
//}

///*
// * mouseMoveEvent
// */
//void dataPlotWidget::mouseMoveEvent( QMouseEvent *event )
//{
//    int pnt = invTransform(QwtPlot::yLeft, (event->pos().y()));

//    if( contrastDragging )
//    {
//        // make sure contrast is always a greater value than brightness
//        if( pnt < currBrightness )
//        {
//            pnt = currBrightness;
//        }

//        // don't go off the bottom and top edges of the graph
//        currContrast = max( pnt, minContrast );
//        currContrast = min( currContrast, maxContrast );

//        emit updateContrast( currContrast );
//    }
//    else if( brightnessDragging )
//    {
//        // make sure brightness is always a smaller value than contrast
//        if( pnt > currContrast )
//        {
//            pnt = currContrast;
//        }

//        // don't go off the bottom and top edges of the graph
//        currBrightness = max( pnt, minBrightness );
//        currBrightness = min( currBrightness, maxBrightness );

//        emit updateBrightness( currBrightness );
//    }
//}

///*
// * mouseReleaseEvent
// */
//void dataPlotWidget::mouseReleaseEvent( QMouseEvent * )
//{
//    // reset the flags
//    brightnessDragging = contrastDragging = false;
//}

///*
// * enableLevels
// */
//void dataPlotWidget::enableLevels( void )
//{
//    showLevels = true;

//    if( contrastCurve )
//    {
//        delete contrastCurve;
//    }
//    contrastCurve = new QwtPlotCurve();
//    contrastCurve->attach( this );

//    if( brightnessCurve )
//    {
//        delete brightnessCurve;
//    }
//    brightnessCurve = new QwtPlotCurve();
//    brightnessCurve->attach( this );

//    contrastCurve->setPen( contrastPen );
//    brightnessCurve->setPen( brightnessPen );

//    QPolygonF contrastPoly;
//    contrastPoly << QPointF( 0.0, currContrast ) << QPointF( this->plotSize, currContrast );
//    contrastCurve->setSamples( contrastPoly );

//    QPolygonF brightnessPoly;
//    brightnessPoly << QPointF( 0.0, currBrightness ) << QPointF( this->plotSize, currBrightness );
//    brightnessCurve->setSamples( brightnessPoly );
//}

///*
// * changeBrightness
// */
//void dataPlotWidget::changeBrightness( int val )
//{
//    currBrightness = val;
//}

///*
// * changeContrast
// */
//void dataPlotWidget::changeContrast( int val )
//{
//    currContrast = val;
//}

///*
// * plotData
// */
//void dataPlotWidget::plotData( unsigned short *data )
//{
//    // Copy data for displaying
//    for( int i = 0; i < plotSize; i++ )
//    {
//        d_y[ i ] = (double)data[ i ]; // not memcpy since the data types are different
//    }
//}

///*
// * plotData
// */
//void dataPlotWidget::plotData( unsigned char *data )
//{
//    // Copy data for displaying
//    for( int i = 0; i < plotSize; i++ )
//    {
//        d_y[ i ] = (double)data[ i ]; // not memcpy since the data types are different
//    }
//}

///*
// * clearData
// */
//void dataPlotWidget::clearData( void )
//{
//    // Copy data for displaying
//    for( int i = 0; i < plotSize; i++ )
//    {
//        d_y[ i ] = 0;
//    }
//}

///*
// * setBrightnessLimits
// */
//void dataPlotWidget::setBrightnessLimits( int lower, int upper )
//{
//    minBrightness = lower;
//    maxBrightness = upper;
//}

///*
// * setContrastLimits
// */
//void dataPlotWidget::setContrastLimits( int lower, int upper )
//{
//    minContrast = lower;
//    maxContrast = upper;
//}
