#include "displayOptionsScreen.h"
#include "ui_displayOptionsScreen.h"
#include "Utility/widgetcontainer.h"

DisplayOptionsScreen::DisplayOptionsScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayOptionsScreen)
{
    ui->setupUi(this);
}

DisplayOptionsScreen::~DisplayOptionsScreen()
{
    delete ui;
}

void DisplayOptionsScreen::on_pushButtonDone_clicked()
{
    WidgetContainer::instance()->gotoScreen("startScreen");
}
