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
    explicit levelGauge(QWidget *parent = nullptr);
    ~levelGauge();
    void init(int desiredNumChunks, float defaultVal, QString title, float minVal, float maxVal );
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
    float controlledObjMinVal;
    float controlledObjMaxVal;
    void   updatePosition( void );
};

#endif // LEVELGAUGE_H
