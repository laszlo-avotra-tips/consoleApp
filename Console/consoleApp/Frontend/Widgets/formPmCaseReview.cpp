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
    ui->labelCaseReview->hide();
    ui->labelDuration->hide();
    ui->labelTime->hide();
    ui->horizontalSlider->hide();
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
    ui->labelCaseReview->show();
    LOG1(isVisible);
    if(isVisible){
        ui->framePlayer->hide();
        ui->captureScene->show();
        ui->pushButtonLogo->hide();
        ui->labelCaseReview->move(960,80);
        ui->labelDuration->hide();
        ui->labelTime->hide();
        ui->horizontalSlider->hide();
        ui->labelCaseReview->setStyleSheet("font: 18pt Helvetica Neue; color: rgb(245,196,0);");
    } else {
        ui->framePlayer->show();
        ui->captureScene->hide();
        ui->pushButtonLogo->show();
        ui->labelCaseReview->move(910,75);
        ui->labelDuration->show();
        ui->labelTime->show();
        ui->horizontalSlider->show();
        //font: 22pt "Helvetica Neue";
        ui->labelCaseReview->setStyleSheet("font: 18pt Helvetica Neue; color: rgb(245,196,0);");
    }
}

QVBoxLayout *FormPmCaseReview::getVideoWidgetContainer()
{
    return ui->verticalLayout;
}

void FormPmCaseReview::updateSliderLabels(const QString &time, const QString &duration)
{
    ui->labelTime->setText(time);
    ui->labelDuration->setText(duration);
}

void FormPmCaseReview::setSliderPosition(quint64 position)
{
    ui->horizontalSlider->setValue(position);
}

void FormPmCaseReview::setSliderRange(quint64 range)
{
    ui->horizontalSlider->setRange(0, range);
}
