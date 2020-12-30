#include "formSecondMonitor.h"
#include "ui_formSecondMonitor.h"

FormSecondMonitor::FormSecondMonitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSecondMonitor)
{
    ui->setupUi(this);
}

FormSecondMonitor::~FormSecondMonitor()
{
    delete ui;
}
