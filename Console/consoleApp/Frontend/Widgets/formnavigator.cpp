#include "formnavigator.h"
#include "ui_formnavigator.h"
#include "Utility/widgetcontainer.h"
#include "Utility/pagefactory.h"
#include <QDebug>
#include <QFile>


FormNavigator::FormNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormNavigator)
{
    ui->setupUi(this);

    auto* stack = ui->stackedWidget;

    auto* widgets = WidgetContainer::instance();

    widgets->setStackedWidget(stack);
    widgets->setNavigator(this);
    PageFactory pf;

    int count = stack->count();

    qDebug() << "page count = " << count;

    widgets->gotoPage("startPage");
}

FormNavigator::~FormNavigator()
{
    delete ui;
}

void FormNavigator::display()
{
    const bool isFullScreen = WidgetContainer::instance()->isFullScreen();

    if(isFullScreen){
        showFullScreen();
    } else {
        show();
    }
}

void FormNavigator::setStylesheet()
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
