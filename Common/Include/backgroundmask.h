/*
 * backgroundmask.h
 *
 * Backgroundmask places a simple QLabel that provides a shading effect to the background of
 * widgets. This produces what is called a "lightbox" effect that dims the background and
 * creates contrast to the modal, focus widget.
 *
 * Author: Ryan Radjabi
 * Copyright (c) 2016-2017 Avinger, Inc.
 */
#ifndef BACKGROUNDMASK_H
#define BACKGROUNDMASK_H

#include <QWidget>
#include <QLabel>

class backgroundMask : public QWidget
{
    Q_OBJECT
public:
    explicit backgroundMask( QWidget *parent = 0 );
    ~backgroundMask();    
    void init( QWidget *parent );

private:
    QLabel *mask;
};

#endif // BACKGROUNDMASK_H
