#include "opaqueScreen.h"
#include "ui_opaqueScreen.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

OpaqueScreen::OpaqueScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpacScreen)
{
    ui->setupUi(this);
    init();
}

OpaqueScreen::~OpaqueScreen()
{
    delete ui;
}

void OpaqueScreen::init()
{
    int duration_ms=500;
    QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(this);
    QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, "opacity");
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    setGraphicsEffect(showing_effect);
    animation->setStartValue(1);
    animation->setEndValue(0.65);
    animation->setDuration(duration_ms);
    group->addAnimation(animation);
    group->start();
}