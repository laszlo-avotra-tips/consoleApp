#include "reviewAndSettingsDialog.h"
#include "ui_reviewAndSettingsDialog.h"

#include <QTimer>

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

void ReviewAndSettingsDialog::showLastButtonSelected(QPushButton *button, bool isChecked)
{
    for( auto* button : m_selectionButtons){
        button->setStyleSheet("background-color:black;");
    }
    ui->frameNext->setStyleSheet("");
    if(isChecked){
        button->setStyleSheet("background-color:#636363;");
        ui->frameNext->setStyleSheet("background-color: rgb(245,196,0); color: black");
    }
}
