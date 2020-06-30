#include "reviewAndSettingsDialog.h"
#include "ui_reviewAndSettingsDialog.h"

ReviewAndSettingsDialog::ReviewAndSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewAndSettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    connect(ui->pushButtonNext, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButtonExit, &QPushButton::clicked, this, &QDialog::reject);

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
