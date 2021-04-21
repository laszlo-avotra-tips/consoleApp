#ifndef SCREENFACTORY_H
#define SCREENFACTORY_H

#include <QString>


class QWidget;

class ScreenFactory
{
public:
    ScreenFactory();
    void registerScreens();
    void unRegisterScreens();

};

#endif // SCREENFACTORY_H
