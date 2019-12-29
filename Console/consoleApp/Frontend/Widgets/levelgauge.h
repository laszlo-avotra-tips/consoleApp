/*
 * levelgauge.h
 *
 * Author: Ryan Radjabi
 *
 * Copyright (c) 2016-2018 Avinger, Inc.
 */
#ifndef LEVELGAUGE_H
#define LEVELGAUGE_H

#include <QWidget>
#include <QString>

namespace Ui {
class levelGauge;
}

class levelGauge : public QWidget
{
    Q_OBJECT
    
public:
    explicit levelGauge(QWidget *parent = 0);
    ~levelGauge();
    void init( int desiredNumChunks, double defaultVal, QString title, double minVal, double maxVal );
    void enableControls( bool val );
    void setValue( double val );

signals:
    void valueChanged( double val );
    
private slots:
    void on_leftButton_clicked();
    void on_rightButton_clicked();

private:
    Ui::levelGauge *ui;
    int    position;
    int    numChunks;
    float  scaleFactor;
    double controlledObjMinVal;
    double controlledObjMaxVal;
    void   updatePosition( void );
};

#endif // LEVELGAUGE_H
