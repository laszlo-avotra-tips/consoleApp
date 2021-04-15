#include "endCaseDialog.h"
#include "ui_endCaseDialog.h"

endCaseDialog::endCaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::endCaseDialog)
{
    ui->setupUi(this);
}

endCaseDialog::~endCaseDialog()
{
    delete ui;
}
