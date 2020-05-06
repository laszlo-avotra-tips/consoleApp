#include "forml300.h"
#include "ui_forml300.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSlider>
#include <signalmodel.h>

FormL300::FormL300(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormL300)
{
    ui->setupUi(this);
    m_graphicsView = ui->graphicsView;
    hideButtons();
}

void FormL300::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
    }
}

FormL300::~FormL300()
{
    delete ui;
}

bool FormL300::isVisible() const
{
    const auto& region = m_graphicsView->visibleRegion();
    return !region.isEmpty();
}

void FormL300::on_pushButton_clicked()
{
    hide();
}

void FormL300::animateHide(QWidget *wid, QParallelAnimationGroup *group)
{
    int duration_ms=1500;
    QGraphicsOpacityEffect * hiding_effect = new QGraphicsOpacityEffect(wid);
    QPropertyAnimation* animation = new QPropertyAnimation(hiding_effect, "opacity");

    wid->setGraphicsEffect(hiding_effect);
    animation->setStartValue(0.5);
    animation->setEndValue(0);
    animation->setDuration(duration_ms);
    group->addAnimation(animation);
}

void FormL300::animateShow(QWidget *wid, QParallelAnimationGroup *group, float opacity)
{
    int duration_ms=1500;
    QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(wid);
    QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, "opacity");

    wid->setGraphicsEffect(showing_effect);
    animation->setStartValue(0);
    animation->setEndValue(opacity);
    animation->setDuration(duration_ms);
    group->addAnimation(animation);
}

void FormL300::hideButtons()
{
    QParallelAnimationGroup *group = new QParallelAnimationGroup(ui->pushButtonMeasure);
    animateHide(ui->pushButton, group);
    animateHide(ui->pushButtonMeasure, group);
    animateHide(ui->pushButtonRecord, group);
    animateHide(ui->pushButtonZoomIn, group);
    animateHide(ui->pushButtonZoomOut, group);
    animateHide(ui->horizontalSliderZoom, group);
    animateHide(ui->pushButtonFlip, group);
    animateHide(ui->pushButtonCamera, group);
    animateShow(ui->pushButtonAvinger,group);
    group->start();
}

void FormL300::on_pushButtonAvinger_toggled(bool checked)
{
    QParallelAnimationGroup *group = new QParallelAnimationGroup(ui->pushButtonMeasure);
    if (checked)
    {
        animateShow(ui->pushButton, group);
        animateShow(ui->pushButtonMeasure, group);
        animateShow(ui->pushButtonRecord, group);
        animateShow(ui->pushButtonZoomIn, group);
        animateShow(ui->pushButtonZoomOut, group);
        animateShow(ui->horizontalSliderZoom, group);
        animateShow(ui->pushButtonFlip, group);
        animateShow(ui->pushButtonCamera, group);
    } else {
        animateHide(ui->pushButton, group);
        animateHide(ui->pushButtonMeasure, group);
        animateHide(ui->pushButtonRecord, group);
        animateHide(ui->pushButtonZoomIn, group);
        animateHide(ui->pushButtonZoomOut, group);
        animateHide(ui->horizontalSliderZoom, group);
        animateHide(ui->pushButtonFlip, group);
        animateHide(ui->pushButtonCamera, group);
    }
    group->start();
}

void FormL300::on_pushButtonZoomIn_clicked()
{
    auto* slider = ui->horizontalSliderZoom;
    int value = slider->value();
    const int maxValue{int(m_imagingDepth.size())};
    if(value < maxValue){
        slider->setValue(value + 1);
    }
}

void FormL300::on_pushButtonZoomOut_clicked()
{
    auto* slider = ui->horizontalSliderZoom;
    int value = slider->value();
    const int minValue{0};
    if(value > minValue){
        slider->setValue(value - 1);
    }
}

void FormL300::on_horizontalSliderZoom_valueChanged(int value)
{
    int depth = m_imagingDepth[value];
    SignalModel::instance()->setImagingDepth_S(depth);
    emit depthChanged(double(depth));
}
