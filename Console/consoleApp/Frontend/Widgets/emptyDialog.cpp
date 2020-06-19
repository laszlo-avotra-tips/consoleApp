#include "emptyDialog.h"
#include "ui_emptyDialog.h"

EmptyDialog::EmptyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmptyDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    connect(ui->pushButton, &QPushButton::clicked, this, &QDialog::accept);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

}

EmptyDialog::~EmptyDialog()
{
    delete ui;
}
