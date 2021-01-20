#include "formSecondMonitor.h"
#include "ui_formSecondMonitor.h"
#include "livescene.h"

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

void FormSecondMonitor::setScene(liveScene *scene)
{
    ui->graphicsView->setScene(scene);
}
