#include "opacScreen.h"
#include "ui_opacscreen.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

OpacScreen::OpacScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpacScreen)
{
    ui->setupUi(this);
    init();
}

OpacScreen::~OpacScreen()
{
    delete ui;
}

void OpacScreen::init()
{
    int duration_ms=1500;
    QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(this);
    QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, "opacity");
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    setGraphicsEffect(showing_effect);
    animation->setStartValue(1.0);
    animation->setEndValue(0.3);
    animation->setDuration(duration_ms);
    group->addAnimation(animation);
}
