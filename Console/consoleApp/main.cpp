#include "formnavigator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FormNavigator navigator;
    navigator.display(); //navigator.showFullScreen();
    return a.exec();
}
