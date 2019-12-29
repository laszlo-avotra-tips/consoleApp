///*
// * keyboardinputcontext.h
// *
// * On screen, virtual keyboard that is used by homescreen and consoleApp.
// * This project initially was based on the Qt example "Input Panel". This
// * can be found at https://doc.qt.io/archives/4.6/tools-inputpanel.html.
// *
// * Authors: Chris White, Ryan Radjabi
// *
// * Copyright (c) 2013-2017 Avinger, Inc.
// */
//#ifndef KEYBOARDINPUTCONTEXT_H
//#define KEYBOARDINPUTCONTEXT_H

//#include <QInputContext>
//#include "keyboard.h"

//class keyboardInputContext : public QInputContext
//{
//    Q_OBJECT

//public:
//    explicit keyboardInputContext( QObject *parent = 0 );
//    ~keyboardInputContext();
//    bool filterEvent( const QEvent *event );
//    QString identifierName();
//    QString language();
//    void setKeyboardParent( QWidget * );
//    QWidget *keyboardWidget()
//    {
//        return (QWidget *)myKeyboard;
//    }
//    void setKeyboardEmbedded( bool embedded )
//    {
//        myKeyboard->setIgnoreFocusEvents( embedded );
//        isEmbedded = embedded;
//    }

//    bool isComposing() const;
//    void reset() {}
//    void refresh();

//private slots:
//    void sendCharacter( int character );

//public slots:
//    void handleFocusChange( QWidget *oldFocus, QWidget *newFocus );

//private:
//    void updatePosition();

//    bool isEmbedded;
//    keyboard *myKeyboard;
//};

//#endif // KEYBOARDINPUTCONTEXT_H
