#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"
#include "logger.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    m_labels = {ui->labelDr1, ui->labelDr2, ui->labelDr3};

    setWindowFlags(Qt::SplashScreen);

    connect(ui->labelDr1, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedLabel);
    connect(ui->labelDr2, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedLabel);
    connect(ui->labelDr3, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedLabel);

    connect(ui->pushButtonDrDefault, &QPushButton::clicked, this, &PreferencesDialog::setDefaultPhysician);

    initPhysiciansContainer();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::handleSelectedLabel(const QString &name)
{
    LOG1(name)
    for(auto& label : m_labels){
        label->setStyleSheet("color: white");
    }

    const auto& labelIt = m_physiciansContainer.find(name);
    if(labelIt != m_physiciansContainer.end()){
        labelIt->second->setStyleSheet("color: rgb(245,196,0)");
    }
    m_defaultPhysicianCandidate = name;
    ui->pushButtonDrDefault->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
}

void PreferencesDialog::setDefaultPhysician()
{
    ui->labelDrDefault->setText(m_defaultPhysicianCandidate);
    ui->pushButtonDrDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
}

void PreferencesDialog::initPhysiciansContainer()
{
    for(const auto& label : m_labels){
        m_physiciansContainer[label->text()] = label;
    }
}
