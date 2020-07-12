#include "screenFactory.h"
#include "startScreen.h"
#include "widgetcontainer.h"
#include "mainScreen.h"
#include "Frontend/Screens/frontend.h"

#include <logger.h>


ScreenFactory::ScreenFactory()
{
    auto& log = Logger::Instance();
    log.init("ScreenFactory");

    StartScreen* startScreen = new StartScreen();
    WidgetContainer::instance()->registerWidget("startScreen", startScreen);

    frontend* l2500Frontend = new frontend();
    WidgetContainer::instance()->registerWidget("l250Frontend", l2500Frontend);
}
