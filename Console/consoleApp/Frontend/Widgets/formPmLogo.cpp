#include "formPmLogo.h"
#include "ui_formPmLogo.h"

FormPmLogo::FormPmLogo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPmLogo)
{
    ui->setupUi(this);
}

FormPmLogo::~FormPmLogo()
{
    delete ui;
}
