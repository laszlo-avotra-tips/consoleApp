#include "mykeyboard.h"
#include "ui_mykeyboard.h"

MyKeyboard::MyKeyboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MyKeyboard)
{
    ui->setupUi(this);
}

MyKeyboard::~MyKeyboard()
{
    delete ui;
}

QString MyKeyboard::name()
{
    return ui->lineEdit->text();
}

void MyKeyboard::on_pushButtona_clicked()
{
    ui->lineEdit->setText(ui->lineEdit->text() + "a");
}
