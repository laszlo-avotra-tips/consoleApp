/*
 * scrubbar.h
 *
 * Author: Chris White
 *
 * Copyright (c) 2015-2018 Avinger, Inc.
 */
#ifndef SCRUBBAR_H
#define SCRUBBAR_H

#include <QSlider>

class scrubbar : public QSlider
{
    Q_OBJECT
public:
    explicit scrubbar(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SCRUBBAR_H
