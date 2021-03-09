#include <QDebug>
#include <QMouseEvent>

#include "consoleLineEdit.h"

ConsoleLineEdit::ConsoleLineEdit(QWidget *parent):QLineEdit(parent)
{

}

void ConsoleLineEdit::mousePressEvent(QMouseEvent *e)
{
    if(e){
        auto bt = e->buttons();
        if(bt == Qt::LeftButton){
            /*
             * handle this event
             */
            e->accept();
            emit mousePressed();
        }else{
            /*
             * ignore the event; to be handled by the parent
             */
            e->ignore();
        }
    }
}

void ConsoleLineEdit::unmark()
{
    m_isMarked = false;
    setStyleSheet("color: white");
}

bool ConsoleLineEdit::isMarked() const
{
    return m_isMarked;
}

void ConsoleLineEdit::setIsMarked(bool isMarked)
{
    m_isMarked = isMarked;
}

void ConsoleLineEdit::mark()
{
    if(!text().isEmpty()){
        m_isMarked = true;
        setStyleSheet("color: rgb(245,196,0)");
    }
}
