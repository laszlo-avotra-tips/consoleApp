#include "formPmCaseReview.h"
#include "ui_formPmCaseReview.h"

#include <QGraphicsScene>

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
    ui->pushButtonLogo->hide();
}

void FormPmCaseReview::showCapture(bool isVisible)
{
    if(isVisible){
        ui->captureScene->show();
    } else {
        ui->captureScene->hide();
    }
}
