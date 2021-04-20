#include "screenFactory.h"
#include "widgetcontainer.h"
#include "mainScreen.h"
#include "caseReviewScreen.h"
#include "logger.h"


ScreenFactory::ScreenFactory()
{
    auto& log = Logger::Instance();
    log.init("ScreenFactory");
}

void ScreenFactory::registerScreens()
{

    MainScreen* mainScreen = new MainScreen();
    WidgetContainer::instance()->registerWidget("mainScreen", mainScreen);

    CaseReviewScreen* caseReviewScreen = new CaseReviewScreen();
    WidgetContainer::instance()->registerWidget("caseReviewScreen", caseReviewScreen);

}

void ScreenFactory::unRegisterScreens()
{
     WidgetContainer::instance()->unRegisterWidget("mainScreen");
     WidgetContainer::instance()->unRegisterWidget("caseReviewScreen");
}
