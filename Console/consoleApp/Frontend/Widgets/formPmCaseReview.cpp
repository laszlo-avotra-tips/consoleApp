#include "formPmCaseReview.h"
#include "ui_formPmCaseReview.h"
#include <logger.h>

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
    LOG1(isVisible);
    if(isVisible){
        ui->framePlayer->hide();
        ui->captureScene->show();
        ui->pushButtonLogo->hide();
        ui->labelCaseReview->move(960,80);
    } else {
        ui->framePlayer->show();
        ui->captureScene->hide();
        ui->pushButtonLogo->show();
        ui->labelCaseReview->move(1100,34);
    }
}

QVBoxLayout *FormPmCaseReview::getVideoWidgetContainer()
{
    return ui->verticalLayout;
}
