#include "formSecondMonitor.h"
#include "ui_formSecondMonitor.h"
#include "livescene.h"
#include "logger.h"

LiveSceneView::LiveSceneView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSecondMonitor)
{
    ui->setupUi(this);

    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->labelSpeed->hide();
    ui->pushButtonSpeed->hide();

}

LiveSceneView::~LiveSceneView()
{
    delete ui;
}

void LiveSceneView::setScene(liveScene *scene)
{
    ui->graphicsView->setScene(scene);
}

void LiveSceneView::setRuntimeLabel(const QString &msg)
{
    ui->labelRunTime->setText(msg);
}

void LiveSceneView::setCurrentTime(const QString &msg)
{
    ui->labelTime->setText(msg);
}

void LiveSceneView::setDevice(const QString &msg)
{
    ui->labelDevice->setText(msg);
}

void LiveSceneView::setLabelLiveColor(const QString &msg)
{
    ui->labelLive->setStyleSheet(msg);
}

void LiveSceneView::setBorderForRecording(const QString &styleSheet)
{
    LOG1(styleSheet);
    ui->frame->setStyleSheet(styleSheet);
}

void LiveSceneView::setEnableRecording(bool isEnabled)
{
    ui->pushButtonRecord->setEnabled(isEnabled);
}

void LiveSceneView::setCheckRecording(bool isChecked)
{
    ui->pushButtonRecord->setChecked(isChecked);
}

void LiveSceneView::pushButtonRecord_clicked(bool isChecked)
{
    ui->pushButtonRecord->clicked(isChecked);
}

void LiveSceneView::setSpeedVisible(bool isVisible)
{
    ui->labelSpeed->setVisible(isVisible);
    ui->pushButtonSpeed->setVisible(isVisible);
}
