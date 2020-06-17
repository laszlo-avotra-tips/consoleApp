#include "screenNavigator.h"
#include "ui_screenNavigator.h"
#include "Utility/widgetcontainer.h"
#include "Utility/screenFactory.h"
#include <QDebug>
#include <QFile>


ScreenNavigator::ScreenNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenNavigator)
{
    ui->setupUi(this);

    auto* stack = ui->stackedWidget;

    auto* widgets = WidgetContainer::instance();

    widgets->setStackedWidget(stack);
    widgets->setNavigator(this);
    ScreenFactory pf;

    int count = stack->count();

    qDebug() << "page count = " << count;

    widgets->gotoScreen("startScreen");
}

ScreenNavigator::~ScreenNavigator()
{
    delete ui;
}

void ScreenNavigator::display()
{
    const bool isFullScreen = WidgetContainer::instance()->isFullScreen();

    if(isFullScreen){
        showFullScreen();
    } else {
        show();
    }
}

void ScreenNavigator::setStylesheet()
{
    QString fn("/Avinger_System/styleSheet.dat");
    QFile sf(fn);
    if(sf.open(QIODevice::ReadOnly)){
        qDebug() << fn << " open ok";
        QTextStream ts(&sf);
        int size;
        int isFullScreen;
        ts >> size >> isFullScreen;
        qDebug() << fn << " open ok. size = " << size << ", isFullScreen " << isFullScreen;
        WidgetContainer::instance()->setIsFullScreen(isFullScreen);
        sf.close();
    }
}
