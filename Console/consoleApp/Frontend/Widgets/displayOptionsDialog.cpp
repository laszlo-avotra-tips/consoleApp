#include "displayOptionsDialog.h"
#include "ui_displayOptionsDialog.h"

DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DisplayOptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

}

DisplayOptionsDialog::~DisplayOptionsDialog()
{
    delete ui;
}

void DisplayOptionsDialog::on_pushButton_clicked()
{
    accept();
}
