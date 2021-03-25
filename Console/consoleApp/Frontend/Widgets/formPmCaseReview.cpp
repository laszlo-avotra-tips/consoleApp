#include "formPmCaseReview.h"
#include "ui_formPmCaseReview.h"
#include "logger.h"

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
    ui->captureScene->show();
    if(isVisible){
        if(ui->stackedWidget->currentIndex()){
            ui->stackedWidget->setCurrentIndex(0);
            LOG2(isVisible, ui->stackedWidget->currentIndex());
        }
//        ui->captureScene->show();
//        ui->pushButtonLogo->hide();
//        ui->labelCaseReview->move(960,80);
    } else {
        if(!ui->stackedWidget->currentIndex()){
            ui->stackedWidget->setCurrentIndex(1);
            LOG2(isVisible, ui->stackedWidget->currentIndex());
        }
//        ui->captureScene->hide();
//        ui->pushButtonLogo->show();
//        ui->labelCaseReview->move(1100,34);
    }
}

QVBoxLayout *FormPmCaseReview::getVideoWidgetContainer()
{
    return ui->verticalLayout;
}
