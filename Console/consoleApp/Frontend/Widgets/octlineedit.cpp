#include <QDebug>
#include <QMouseEvent>

#include "octlineedit.h"

OctLineEdit::OctLineEdit(QWidget *parent):QLineEdit(parent)
{

}

void OctLineEdit::mousePressEvent(QMouseEvent *e)
{
    auto bt = e->buttons();
    if(bt == Qt::LeftButton){
        e->accept();
        emit mousePressed();
    }else{
        e->ignore();
    }
}
