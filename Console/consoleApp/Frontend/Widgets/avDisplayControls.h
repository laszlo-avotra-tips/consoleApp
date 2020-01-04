/*
 * avDisplayControls.h
 *
 * Dual-handled slider to allow users to change the brightness and contrast
 * settings with one control since the brightness must always be less than
 * or equal to the contrast.
 *
 * Author: Dennis W. Jackson
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 *
 */
#ifndef AVDISPLAYCONTROLS_H
#define AVDISPLAYCONTROLS_H

//#include <QxtSpanSlider>
#include <QSlider>

class avDisplayControls : public QSlider
{
    Q_OBJECT

public:
    avDisplayControls( QWidget * = nullptr );

signals:

public slots:
    void updateBrightnessContrastLimits();

protected:

private:
    const QColor SpanEnabledColor { 191, 191, 191 };
    const QColor SpanDisabledColor { 128, 128, 128 };

};

#endif // AVDISPLAYCONTROLS_H
