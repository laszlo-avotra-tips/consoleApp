/*
 * keyboard.cpp
 *
 * On screen, virtual keyboard that is used by homescreen and consoleApp.
 * This project initially was based on the Qt example "Input Panel". This
 * can be found at https://doc.qt.io/archives/4.6/tools-inputpanel.html.
 *
 * Authors: Chris White, Ryan Radjabi
 *
 * Copyright (c) 2013-2018 Avinger, Inc.
 */
#include "keyboard.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

const QString closeKeyStyle( "QPushButton { min-width: 30; background: #8F2400; color: black; border-radius: 5; } QPushButton:Pressed { background: #7F2400; color: black; border-radius: 5; }" );
const QString keyboardStyle( "QWidget { background: #444444; font: 12pt DINPro-Medium; }\n QPushButton { min-width: 0; background: #aaaaaa; color: black; border-radius: 10; }\n QPushButton:Pressed { background: #888888; color: white; border: 2px solid white; border-radius: 10 }" );

const int DefaultKeyWidth( 120 );
const int DefaultKeyHeight( 50 );

/*
 * constructor
 */
keyboard::keyboard()
    : QWidget( 0, ( Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ) ),
    lastFocusedWidget( 0 )
{
    setFocusPolicy( Qt::NoFocus );
    signalMapper = new QSignalMapper( this );
    lastClickedButton = NULL;

    setStyleSheet( keyboardStyle );

    buildKeyboard();
    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), this, SLOT( saveFocusWidget( QWidget*, QWidget* ) ) );
    connect( signalMapper, SIGNAL( mapped( QWidget* ) ),         this, SLOT( buttonClicked( QWidget* ) ) );
    setMaximumWidth(1700);
    lower = true;
}

/*
 * buildKey
 *
 * Create a single key on the keyboard. Select
 * on the key type to change dimensions and other
 * appearance parameters.
 */
QPushButton *keyboard::buildKey( QString keyName,
                                 int key,
                                 bool alpha,
                                 int scaleWidth,
                                 int scaleHeight )
{
    keyPushButton *keyButton = new keyPushButton( this );

    keyButton->setText( keyName );
    keyButton->setObjectName( keyName );
    keyButton->setFocusPolicy( Qt::NoFocus );

    // Set style sheet
    keyButton->setMaximumSize( DefaultKeyWidth * scaleWidth,
                               DefaultKeyHeight * scaleHeight );
    keyButton->setMinimumSize( DefaultKeyWidth * scaleWidth,
                               DefaultKeyHeight * scaleHeight );
    keyButton->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

    // Map signals
    keyButton->setProperty( "buttonValue", key );

    /*
     * Assign a lower case value for alpha. This will be +32 in ASCII.
     * Otherwise, assign the same key for lower.
     */
    if( alpha && ( key >= Qt::Key_A ) && ( key <= Qt::Key_Z ) )
    {
        keyButton->setProperty( "buttonValueLower", ( key + 32 ) );
    }
    else
    {
        keyButton->setProperty( "buttonValueLower", key );
    }
    signalMapper->setMapping( keyButton, keyButton );
    connect( keyButton, SIGNAL( clicked() ), signalMapper, SLOT( map() ) );
    return( keyButton );
}

/*
 * buildCloseKey
 *
 * Place a small "close" button in the upper right corner.
 */
QPushButton *keyboard::buildCloseKey( void )
{
    QString keyName = "close";
    int scaleWidth = 1;
    int scaleHeight = 1;

    keyPushButton *keyButton = new keyPushButton( this );

    keyButton->setText( QString( "x" ) );
    keyButton->setObjectName( keyName );
    keyButton->setFocusPolicy( Qt::NoFocus );

    keyButton->setStyleSheet( closeKeyStyle );

    // Set style sheet
    keyButton->setMaximumSize( 20 * scaleWidth,
                               20 * scaleHeight );
    keyButton->setMinimumSize( 20 * scaleWidth,
                               20 * scaleHeight );
    keyButton->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

    // Map signals
    connect( keyButton, SIGNAL( clicked() ), this, SLOT( dismissKeyboard() ) );
    return( keyButton );
}


/*
 * buttonClicked
 */
void keyboard::buttonClicked( QWidget *w )
{
    int chr;
    if( lower )
    {
        chr = qvariant_cast<int>( w->property( "buttonValueLower" ) );
    }
    else
    {
        chr = qvariant_cast<int>( w->property( "buttonValue" ) );
    }

    if( chr == Qt::Key_Shift )
    {
        lower = !lower; // toggle caps

        lastClickedButton = (QPushButton *)w; // cache the last button
        if( !lower )
        {
            setKeyStyleClicked();
        }
        else
        {
            resetKeyStyle();
        }
        return; // escape here for shift key
    }

    // reset caps
    if( !lower )
    {
       lower = true;
    }

    // reset last key before servicing the next key press
    resetKeyStyle();

    emit keyPressed( chr );

    lastClickedButton = (QPushButton *)w;                     // cache the last button
    setKeyStyleClicked();                                     // set the styleSheet for the last button
    QTimer::singleShot( 200, this, SLOT( resetKeyStyle() ) ); // reset the stylesheet after 200 ms timeout
}

/*
 * setKeyStyleClicked
 */
void keyboard::setKeyStyleClicked()
{
    if( lastClickedButton != NULL )
    {
        lastClickedButton->setStyleSheet( "QPushButton{ background: #888888; color: white; border: 2px solid white; }" );
    }
}

/*
 * resetKeyStyle
 */
void keyboard::resetKeyStyle()
{
    if( lastClickedButton != NULL )
    {
        lastClickedButton->setStyleSheet( "QPushButton{ color: black; }" );
        shiftButton->setStyleSheet( "QPushButton{ color: black; }" );
    }
}

/*
 * buildKeyboard
 *
 * Build the entire keyboard, including each individual
 * key, the layouts that contain them, etc.
 */
void keyboard::buildKeyboard( void )
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    QStringList keyList;

    for( char i = '1'; i <= '9'; i++ )
    {
        hLayout->addWidget( buildKey( QChar( i ), i ), 2 );
    }

    hLayout->addWidget( buildKey( "0", '0' ), 2 );
    hLayout->addWidget( buildKey( "-", '-' ), 2 );
    hLayout->addWidget( buildKey( "delete", Qt::Key_Backspace, false, 2 ), 3 );
    hLayout->addSpacing( 20 );
    hLayout->addWidget( buildCloseKey(), 3 );
    vLayout->insertLayout( 0, hLayout );

    // Next row, first row of letters
    hLayout = new QHBoxLayout;
    QVBoxLayout *tmpVLayout = new QVBoxLayout;

    keyList << "Q" << "W" << "E" << "R" << "T" << "Y"
            << "U" << "I" << "O" << "P" << "[" << "]" << "\\";

    QString keyLabel;
    foreach( keyLabel, keyList )
    {
        hLayout->addWidget( buildKey( keyLabel, keyLabel[0].toLatin1(), true ), 2 );
    }
    tmpVLayout->insertLayout( 0, hLayout );

    // Second row of letters
    keyList.clear();
    hLayout = new QHBoxLayout;

    keyList << "A" << "S" << "D" << "F" << "G" << "H"
            << "J" << "K" << "L" << ";" << "'";

    hLayout->addSpacing(30);
    foreach( keyLabel, keyList )
    {
        hLayout->addWidget( buildKey( keyLabel, keyLabel[0].toLatin1(), true ), 2 );
    }

    tmpVLayout->insertLayout( 1, hLayout, 1 );

    hLayout = new QHBoxLayout;
    hLayout->insertLayout(0, tmpVLayout, 1);
    hLayout->addWidget( buildKey( "return", Qt::Key_Return, false, 1, 2 ), 3 );
    vLayout->insertLayout( 2, hLayout );

    // Third row of letters
    keyList.clear();
    hLayout = new QHBoxLayout;


    hLayout->addSpacing(20);
    shiftButton = buildKey( "shift", Qt::Key_Shift, false, 4, 1);
    hLayout->addWidget( shiftButton, 3 );

    keyList << "Z" << "X" << "C" << "V" << "B" << "N"
            << "M" << "," << "." << "/";

    foreach( keyLabel, keyList )
    {
        hLayout->addWidget( buildKey( keyLabel, keyLabel[0].toLatin1(), true ), 2 );
    }
    hLayout->addSpacing(50);
    vLayout->insertLayout( 3, hLayout );

    // Last row
    keyList.clear();
    hLayout = new QHBoxLayout;
    keyList << "<" << ">" << "?";
    foreach( keyLabel, keyList )
    {
        hLayout->addWidget( buildKey( keyLabel, keyLabel[0].toLatin1() ), 2 );
    }
    hLayout->addWidget( buildKey( "space", Qt::Key_Space, false, 9 ), 2 );
    keyList.clear();
    keyList << ":" << "\"" << "*";
    foreach( keyLabel, keyList )
    {
        hLayout->addWidget( buildKey( keyLabel, keyLabel[0].toLatin1() ), 2 );
    }

    vLayout->insertLayout( 4, hLayout );
    this->setLayout( vLayout );
}

/*
 * setIgnoreFocusEvents
 */
void keyboard::setIgnoreFocusEvents( bool ignore)
{
    ignoreFocusEvents = ignore;
}

/*
 * event
 */
bool keyboard::event( QEvent *e )
 {
    if( !ignoreFocusEvents )
    {
        switch( e->type() )
        {
        case QEvent::WindowActivate:
            if( lastFocusedWidget )
            {
                lastFocusedWidget->activateWindow();
            }
            break;
        default:
            break;
        }
    }
    return QWidget::event( e );
 }

/*
 * saveFocusWidget
 */
void keyboard::saveFocusWidget( QWidget * /*oldFocus*/, QWidget *newFocus )
{
    if( !ignoreFocusEvents )
    {
        if( newFocus != 0 && !this->isAncestorOf( newFocus ) )
        {
            lastFocusedWidget = newFocus;
        }
    }
}

/*
 * dismissKeyboard
 */
void keyboard::dismissKeyboard()
{
    this->hide();
}
