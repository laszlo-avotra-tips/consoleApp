#include "screenFactory.h"
#include "startScreen.h"
#include "widgetcontainer.h"
#include "mainScreen.h"
#include "logger.h"


ScreenFactory::ScreenFactory()
{
    auto& log = Logger::Instance();
    log.init("ScreenFactory");

    StartScreen* startScreen = new StartScreen();
    WidgetContainer::instance()->registerWidget("startScreen", startScreen);

    MainScreen* mainScreen = new MainScreen();
    WidgetContainer::instance()->registerWidget("mainScreen", mainScreen);

}
