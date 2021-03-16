#include "shutdownConfirmationDialog.h"
#include "ui_shutdownConfirmationDialog.h"

ShutdownConfirmationDialog::ShutdownConfirmationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShutdownConfirmationDialog)
{
    ui->setupUi(this);
}

ShutdownConfirmationDialog::~ShutdownConfirmationDialog()
{
    delete ui;
}
