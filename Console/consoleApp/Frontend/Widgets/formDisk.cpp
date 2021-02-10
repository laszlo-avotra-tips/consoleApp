#include "formDisk.h"
#include "ui_formDisk.h"

FormDisk::FormDisk(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormDisk)
{
    ui->setupUi(this);
}

FormDisk::~FormDisk()
{
    delete ui;
}
