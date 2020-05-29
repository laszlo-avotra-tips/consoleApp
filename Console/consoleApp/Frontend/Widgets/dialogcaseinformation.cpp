#include "dialogcaseinformation.h"
#include "ui_dialogcaseinformation.h"

DialogCaseInformation::DialogCaseInformation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCaseInformation)
{
    ui->setupUi(this);
}

DialogCaseInformation::~DialogCaseInformation()
{
    delete ui;
}
