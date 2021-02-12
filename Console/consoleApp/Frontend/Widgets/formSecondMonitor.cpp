#include "formSecondMonitor.h"
#include "ui_formSecondMonitor.h"
#include "livescene.h"

LiveSceneView::LiveSceneView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSecondMonitor)
{
    ui->setupUi(this);
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
