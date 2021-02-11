#include "formDisk.h"
#include "ui_formDisk.h"

FormDisk::FormDisk(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormDisk)
{
    ui->setupUi(this);
    connect(this, &QWidget::windowTitleChanged, this, &FormDisk::setInfoLabel);
}

FormDisk::~FormDisk()
{
    delete ui;
}

void FormDisk::setInfoLabel(const QString &msg)
{
    ui->labelInfo->setText(msg);
}
