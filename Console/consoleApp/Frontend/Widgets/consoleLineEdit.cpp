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
