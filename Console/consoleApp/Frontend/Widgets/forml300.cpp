#include "forml300.h"
#include "ui_forml300.h"

FormL300::FormL300(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormL300)
{
    ui->setupUi(this);
}

FormL300::~FormL300()
{
    delete ui;
}

void FormL300::on_pushButton_clicked()
{
    hide();
}
