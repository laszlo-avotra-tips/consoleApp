#include "dialogdeviceselect.h"
#include "ui_dialogdeviceselect.h"
#include "Frontend/Utility/widgetcontainer.h"

DialogDeviceSelect::DialogDeviceSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDeviceSelect)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
}

DialogDeviceSelect::~DialogDeviceSelect()
{
    delete ui;
}

void DialogDeviceSelect::on_pushButtonDone_clicked()
{
//    WidgetContainer::instance()->gotoPage("mainPage");
}
