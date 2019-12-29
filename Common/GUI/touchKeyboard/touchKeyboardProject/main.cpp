#include <QtGui/QApplication>
#include "mainwindow.h"
#include "keyboardinputcontext.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    keyboardInputContext *ic = new keyboardInputContext;
    a.setInputContext(ic);
    MainWindow w;
    w.show();

    return a.exec();
}
