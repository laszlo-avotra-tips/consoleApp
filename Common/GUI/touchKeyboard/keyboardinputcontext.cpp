///*
// * keyboardinputcontext.cpp
// *
// * On screen, virtual keyboard that is used by homescreen and consoleApp.
// * This project initially was based on the Qt example "Input Panel". This
// * can be found at https://doc.qt.io/archives/4.6/tools-inputpanel.html.
// *
// * Authors: Chris White, Ryan Radjabi
// *
// * Copyright (c) 2013-2018 Avinger, Inc.
// */
//#include <QApplication>
//#include "keyboardinputcontext.h"
//#include "windowmanager.h"
//#include "defaults.h"

///*
// * Constructor
// */
//keyboardInputContext::keyboardInputContext( QObject *parent ) :
//    QInputContext( parent )
//{
//    myKeyboard = new keyboard;
//    isEmbedded = false;
//    connect( myKeyboard, SIGNAL( keyPressed(int) ), SLOT( sendCharacter(int) ) );
//    connect( qApp, SIGNAL( focusChanged(QWidget*,QWidget*) ), this, SLOT( handleFocusChange(QWidget*,QWidget*) ) );
//}

///*
// * Destructor
// */
//keyboardInputContext::~keyboardInputContext()
//{
//}

///*
// * setKeyboardParent
// */
//void keyboardInputContext::setKeyboardParent( QWidget *widget )
//{
//    myKeyboard->setParent( widget );
//}

///*
// * identifierName
// */
//QString keyboardInputContext::identifierName()
//{
//    return( "keyboardInputContext" );
//}

///*
// * refresh
// */
//void keyboardInputContext::refresh()
//{
//    delete myKeyboard;
//    isEmbedded = false;
//    myKeyboard = new keyboard;
//    connect( myKeyboard, SIGNAL( keyPressed(int) ), SLOT( sendCharacter(int) ) );
//}

///*
// * isComposing
// */
//bool keyboardInputContext::isComposing() const
//{
//    return false;
//}

///*
// * language
// */
//QString keyboardInputContext::language()
//{
//    return( "en_US" );
//}

///*
// * filterEvent
// */
//bool keyboardInputContext::filterEvent( const QEvent *event )
//{
//    if( !isEmbedded )
//    {
//        if( event->type() == QEvent::RequestSoftwareInputPanel )
//        {
//            updatePosition();
//            myKeyboard->show();
//            return true;
//        }
//        else if( event->type() == QEvent::CloseSoftwareInputPanel )
//        {
//            myKeyboard->hide();
//            return true;
//        }
//    }
//    return false;
//}

///*
// * sendCharacter
// */
//void keyboardInputContext::sendCharacter( int character )
//{
//    QPointer<QWidget> w = focusWidget();

//    if( !w )
//    {
//        return;
//    }

//    QKeyEvent keyPress( QEvent::KeyPress, character, Qt::NoModifier, QString(character) );
//    QApplication::sendEvent( w, &keyPress );

//    if( !w )
//    {
//        return;
//    }

//    QKeyEvent keyRelease( QEvent::KeyRelease, character, Qt::NoModifier, QString() );
//    QApplication::sendEvent(w, &keyRelease);
//}

///*
// * updatePosition
// *
// * The keyboard should be positioned at the bottom of a modal window without
// * overlapping the window. In the event that an overlap would occur, or the keyboard
// * would run off the edge of the screen, move the keyboard to the bottom of the
// * screen and move the window to the top of the keyboard.
// *
// * If the focus widget is non-modal, then it isn't a dialog that should be moved. In
// * this case, move the keyboard to the bottom of the screen.
// */
//void keyboardInputContext::updatePosition()
//{
//    /*
//     * Unreliable using myKeyboard->width() and height(), so use
//     * hard coded values instead.
//     */
//    const int kbWidth  = 1200;
//    const int kbHeight = 300;
//    WindowManager &wmgr = WindowManager::Instance();
//    const int ScreenWidth  = wmgr.getTechnicianDisplayGeometry().width();
//    const int ScreenHeight = wmgr.getTechnicianDisplayGeometry().height();

//    QWidget *widget = focusWidget();
//    if( !widget )
//    {
//        return;
//    }

//    // get the deepest ancestor of the focus widget
//    widget = widget->window();

//    if( widget->isModal() && !widget->isFullScreen() ) // widget is a dialog and can be moved
//    {
//        // KB below window
//        QPoint kbPos = QPoint( widget->rect().bottomLeft() );
//        kbPos = QPoint( widget->rect().bottomLeft() );
//        kbPos = widget->mapToGlobal( kbPos );
//        myKeyboard->move( kbPos );
//        kbPos.setX( ( ScreenWidth / 2 ) - ( kbWidth / 2 ) ); // Always center horizontally
//        myKeyboard->setGeometry( kbPos.x(), kbPos.y(), kbWidth, kbHeight );

//        // KB off screen?
//        if( kbPos.y() + kbHeight >= ScreenHeight )
//        {
//            // KB to bottom of screen
//            myKeyboard->setGeometry( ( ( ScreenWidth / 2 ) - ( kbWidth / 2 ) ), ( ScreenHeight - kbHeight ), kbWidth, kbHeight );

//            // Move window above KB
//            widget->setGeometry( widget->x(),
//                                 ( ( ScreenHeight - kbHeight ) - widget->height() ),
//                                 widget->width(),
//                                 widget->height() );
//        }
//    }
//    else
//    {
//        // KB to bottom of screen
//        myKeyboard->setGeometry( ( ( ScreenWidth / 2 ) - ( kbWidth / 2 ) ), ( ScreenHeight - kbHeight ), kbWidth, kbHeight );
//    }
//}

///*
// * handleFocusChange
// *
// * Hide the keyboard when it no-longer has focus.
// */
//void keyboardInputContext::handleFocusChange( QWidget *, QWidget *newFocus )
//{
//    if( newFocus == 0 )
//    {
//        myKeyboard->hide();
//    }
//}
