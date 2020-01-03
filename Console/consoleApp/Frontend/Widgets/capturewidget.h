/*
 * captureWidget.h
 *
 * The capture widget is a compound widget consisting of a scrollable
 * list of recent captures and a larger image representing the most
 * recently selected capture.
 *
 * Author: Chris White
 * Copyright (c) 2010-2018 Avinger
 */
#pragma once

#include "Utility/captureListModel.h"
#include "Utility/clipListModel.h"
#include <QWidget>
#include <QItemDelegate>
#include "ui_capturewidget.h"

namespace Ui {
    class captureWidget;
}

/*
 * captureItemDelegate
 *
 * The captureItemDelegate knows how to draw individual thumbnails
 * of captures, along with any tagging or shortcut markup, for inclusion
 * in the recent captures list.
 */
class captureItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    captureItemDelegate(bool rotated = false, QObject *parent = nullptr);
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
    void paint( QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const;
private:
    bool doRotate;
    const QColor SelectedItemColor{ 143, 185, 224 };
};


/*
 * clipItemDelegate
 *
 * The clipItemDelegate knows how to draw individual thumbnails
 * of captures, along with any tagging or shortcut markup, for inclusion
 * in the recent captures list.
 */
class clipItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    clipItemDelegate(bool rotated = false, QObject *parent = nullptr);
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
private:
    bool doRotate;
    const QColor SelectedItemColor{ 143, 185, 224 };
};

/*
 *
 */
class captureWidget : public QWidget
{
    Q_OBJECT

public:
    captureWidget( QWidget *parent = nullptr );
    ~captureWidget();
    void init( void );

signals:
    void showCapture( const QImage &, const QImage &);
    void currentCaptureChanged( QModelIndex );
    void sendLoopFilename( QString );
    void sendStatusText( QString );
    void sendDeviceName( QString );
    void displayingCapture();
    void sendReviewImageCalibrationFactors( int, float );

public slots:
    void captureSelected(QModelIndex ix);
    void displayCapture(QModelIndex index);
    void loopSelected(QModelIndex ix);
    void loadLoop(QModelIndex index);
    void updateCaptureCount( void );
    void updateClipCount( void );
    void disableClipSelection( void ) { ui->loopsListView->setDisabled( true ); }
    void enableClipSelection( void )  { ui->loopsListView->setDisabled( false ); }
    void reviewStateEnded( void );
    bool getReviewState( void ) { return ( isImageReviewInProgress || isLoopReviewInProgress ); }
//UNUSED    int  getPxPerMmConversion( void ) { return selectedCaptureItem->getPixelsPerMm(); }
    void replaceDecoratedImage( QImage p ) { selectedCaptureItem->replaceDecoratedImage( p ); }
    void updateZoomFactor( float value );

protected:
    void changeEvent( QEvent *e );
    bool eventFilter( QObject *obj, QEvent *event );

private slots:
    void on_selectedCaptureLineEdit_editingFinished();
    void on_selectedLoopLineEdit_editingFinished();
    void on_showDecoratedPushButton_toggled(bool checked);

private:
    int  numCaptures;
    int  numLoops;

    bool displayDecoratedImages;
    bool isImageReviewInProgress;
    bool isLoopReviewInProgress;

    captureItem *selectedCaptureItem;
    clipItem    *selectedLoopItem;

    Ui::captureWidget *ui;
};
