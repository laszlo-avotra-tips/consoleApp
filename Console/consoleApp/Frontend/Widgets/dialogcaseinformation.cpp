#include "dialogcaseinformation.h"
#include "ui_dialogcaseinformation.h"

#include <QDateTime>

DialogCaseInformation::DialogCaseInformation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCaseInformation)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}

DialogCaseInformation::~DialogCaseInformation()
{
    delete ui;
}
