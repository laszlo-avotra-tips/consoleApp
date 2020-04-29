#include "forml300.h"
#include "ui_forml300.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

FormL300::FormL300(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormL300)
{
    ui->setupUi(this);
    m_sector = new sectorItem();
    m_scene = new liveScene(this);
    m_graphicsView = ui->graphicsView;
    m_scene->addItem(m_sector);
    m_graphicsView->setScene(m_scene);
}

FormL300::~FormL300()
{
    delete ui;
    delete m_sector;
}

QGraphicsPixmapItem *FormL300::sectorHandle()
{
    return m_sector;
}

QImage *FormL300::sectorImage() const
{
    return m_sector->getSectorImage();
}

bool FormL300::isVisible() const
{
    const auto& region = m_graphicsView->visibleRegion();
    return !region.isEmpty();
}

sectorItem *FormL300::sector()
{
    return m_sector;
}

liveScene *FormL300::scene()
{
    return m_scene;
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
    QGraphicsOpacityEffect * hiding_effect = new QGraphicsOpacityEffect(wid);
    QPropertyAnimation* animation = new QPropertyAnimation(hiding_effect, "opacity");

    wid->setGraphicsEffect(hiding_effect);
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
    animateHide(ui->pushButtonZoom, group);
//    animateHide(ui.endCaseButton, group);
    animateHide(ui->pushButtonCamera, group);
//    animateShow(ui.pushButton_2,group);
    group->start();
}

void FormL300::on_pushButtonAvinger_clicked(bool checked)
{
    QParallelAnimationGroup *group = new QParallelAnimationGroup(ui->pushButtonMeasure);
    if (checked)
    {
        animateShow(ui->pushButton, group, 1.0f);
        animateShow(ui->pushButtonMeasure, group);
        animateShow(ui->pushButtonRecord, group, 1.0f);
        animateShow(ui->pushButtonZoom, group);
//        animateShow(ui.endCaseButton, group);
        animateShow(ui->pushButtonCamera, group);
    } else {
        animateHide(ui->pushButton, group);
        animateHide(ui->pushButtonMeasure, group);
        animateHide(ui->pushButtonRecord, group);
        animateHide(ui->pushButtonZoom, group);
//        animateHide(ui.endCaseButton, group);
        animateHide(ui->pushButtonCamera, group);
    }
    group->start();

}
