/*
 * keyboard.h
 *
 * On screen, virtual keyboard that is used by homescreen and consoleApp.
 * This project initially was based on the Qt example "Input Panel". This
 * can be found at https://doc.qt.io/archives/4.6/tools-inputpanel.html.
 *
 * Authors: Chris White, Ryan Radjabi
 *
 * Copyright (c) 2013-2017 Avinger, Inc.
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QtGui>
#include <QtCore>
#include <QWidget>
#include <QPushButton>
#include <QSignalMapper>

class keyPushButton : public QPushButton
{
    Q_OBJECT
public:
    keyPushButton( QWidget *parent ) : QPushButton( parent ) { }
};

class keyboard : public QWidget
{
    Q_OBJECT
public:
    explicit keyboard();
    void buildKeyboard( void );

signals:
    void keyPressed( int );

public slots:
    void buttonClicked( QWidget * );
    void setIgnoreFocusEvents( bool );

protected:
    bool event( QEvent *e );

private slots:
    void saveFocusWidget( QWidget *oldFocus, QWidget *newFocus );
    void dismissKeyboard();
    void resetKeyStyle();
    void setKeyStyleClicked();

private:
    QPushButton *buildKey( QString keyName,
                           int key,
                           bool alpha = false,
                           int scaleWidth = 1,
                           int scaleHeight = 1 );
    QPushButton *buildCloseKey( void );
    QPushButton *lastClickedButton;
    QPushButton *shiftButton;

    QSignalMapper *signalMapper;
    QWidget *lastFocusedWidget;
    bool ignoreFocusEvents;
    bool lower;
};

#endif // KEYBOARD_H
