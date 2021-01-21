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
