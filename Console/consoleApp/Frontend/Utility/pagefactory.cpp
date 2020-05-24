#include "pagefactory.h"
#include "formstart.h"
#include "widgetcontainer.h"
#include "mainwindow.h"
#include <logger.h>

PageFactory::PageFactory()
{
    auto& log = Logger::Instance();
    log.init("frame");

    MainWindow* mainWindow = new MainWindow();
    WidgetContainer::instance()->registerWidget("mainPage", mainWindow);

    FormStart* formStart = new FormStart();
    WidgetContainer::instance()->registerWidget("startPage", formStart);
}
