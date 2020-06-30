#include "reviewAndSettingsDialog.h"
#include "ui_reviewAndSettingsDialog.h"

ReviewAndSettingsDialog::ReviewAndSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewAndSettingsDialog)
{
    ui->setupUi(this);
}

ReviewAndSettingsDialog::~ReviewAndSettingsDialog()
{
    delete ui;
}

void ReviewAndSettingsDialog::on_pushButtonExit_clicked()
{
    reject();
}

void ReviewAndSettingsDialog::on_pushButtonNext_clicked()
{
    accept();
}
