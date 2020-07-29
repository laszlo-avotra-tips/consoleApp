#include "reviewAndSettingsDialog.h"
#include "ui_reviewAndSettingsDialog.h"

#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

ReviewAndSettingsDialog::ReviewAndSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewAndSettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    connect(ui->pushButtonNext, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButtonExit, &QPushButton::clicked, this, &QDialog::reject);

    std::vector<QPushButton*> buttons{
        ui->pushButtonCaseReview,
        ui->pushButtonDeviceSelect,
        ui->pushButtonDisplayOptions,
        ui->pushButtonCaseInformation
    };

    m_selectionButtons = buttons;
    ui->pushButtonNext->setEnabled(false);

    const bool isAnimation{true};
    if(isAnimation){
        int duration_ms=1000;
        QGraphicsOpacityEffect * showing_effect = new QGraphicsOpacityEffect(this);
        QPropertyAnimation* animation = new QPropertyAnimation(showing_effect, "opacity");
        QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

        setGraphicsEffect(showing_effect);
        animation->setStartValue(0);
        animation->setEndValue(0.9);
        animation->setDuration(duration_ms);
        group->addAnimation(animation);
        group->start();
    }

}

ReviewAndSettingsDialog::~ReviewAndSettingsDialog()
{
    delete ui;
}

void ReviewAndSettingsDialog::on_pushButtonExit_clicked()
{
    reject();
}

void ReviewAndSettingsDialog::on_pushButtonNext_clicked()
{
    accept();
}

void ReviewAndSettingsDialog::on_pushButtonDisplayOptions_clicked(bool checked)
{
   showLastButtonSelected(ui->pushButtonDisplayOptions, checked);
}

void ReviewAndSettingsDialog::on_pushButtonCaseReview_clicked(bool checked)
{
    showLastButtonSelected(ui->pushButtonCaseReview, checked);
}

void ReviewAndSettingsDialog::on_pushButtonDeviceSelect_clicked(bool checked)
{
    showLastButtonSelected(ui->pushButtonDeviceSelect, checked);
}

void ReviewAndSettingsDialog::on_pushButtonCaseInformation_clicked(bool checked)
{
    showLastButtonSelected(ui->pushButtonCaseInformation, checked);
}

void ReviewAndSettingsDialog::showLastButtonSelected(QPushButton *button, bool)
{
    for( auto* btt : m_selectionButtons){
        if(button != btt){
            btt->setStyleSheet("background-color:black;");
        }
    }

    button->setStyleSheet("background-color:#646464;  border-radius: 20px solid grey;");
    ui->frameNext->setStyleSheet("background-color: rgb(245,196,0); color: black");
    ui->pushButtonNext->setEnabled(true);
    m_selection = button->text();

    //pass the selection in the window title
    setWindowTitle(m_selection);
}
