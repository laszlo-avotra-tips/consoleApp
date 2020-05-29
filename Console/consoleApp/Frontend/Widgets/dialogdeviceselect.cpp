#include "dialogdeviceselect.h"
#include "ui_dialogdeviceselect.h"

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
