#include "formPmCaseReview.h"
#include "ui_formPmCaseReview.h"

#include <QGraphicsScene>
#include <QLabel>

FormPmCaseReview::FormPmCaseReview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPmCaseReview)
{
    ui->setupUi(this);
}

FormPmCaseReview::~FormPmCaseReview()
{
    delete ui;
}

void FormPmCaseReview::setScene(QGraphicsScene* scene)
{
    ui->captureScene->setScene(scene);
}

void FormPmCaseReview::showCapture(bool isVisible)
{
    if(isVisible){
        ui->captureScene->show();
        ui->pushButtonLogo->hide();
        ui->labelRunTime->move(960,80);
    } else {
        ui->captureScene->hide();
        ui->pushButtonLogo->show();
        ui->labelRunTime->move(1100,34);
    }
}
