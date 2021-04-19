#include "endCaseDialog.h"
#include "ui_endCaseDialog.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "displayManager.h"

endCaseDialog::endCaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::endCaseDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    /*
     * Set opacity with animation
     */
    const int animationDuration_ms=1000;
    const QByteArray property{"opacity"};
    const float startValue{0.0f};
    const float endValue{0.9f};

    QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(this);
    QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, property);
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);


    setGraphicsEffect(showing_effect);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setDuration(animationDuration_ms);
    group->addAnimation(animation);
    group->start();

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );
    DisplayManager::instance()->showOnTheSecondMonitor("disk");
    DisplayManager::instance()->setWindowTitle("END CASE IN PROCESS");

}

endCaseDialog::~endCaseDialog()
{
    delete ui;
}
