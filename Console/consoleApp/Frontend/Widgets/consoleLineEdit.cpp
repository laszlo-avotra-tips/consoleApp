#include <QDebug>
#include <QMouseEvent>

#include "consoleLineEdit.h"

ConsoleLineEdit::ConsoleLineEdit(QWidget *parent):QLineEdit(parent)
{

}

void ConsoleLineEdit::mousePressEvent(QMouseEvent *e)
{
    auto bt = e->buttons();
    if(bt == Qt::LeftButton){
        e->accept();
        emit mousePressed();
    }else{
        e->ignore();
    }
}
