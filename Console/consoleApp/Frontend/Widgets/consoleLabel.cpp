#include "consoleLabel.h"
#include <QMouseEvent>

ConsoleLabel::ConsoleLabel(QWidget *parent):QLabel(parent)
{

}

void ConsoleLabel::mousePressEvent(QMouseEvent *e)
{
    auto bt = e->buttons();
    if(bt == Qt::LeftButton){
        e->accept();
        emit mousePressed();
    }else{
        e->ignore();
    }
}
