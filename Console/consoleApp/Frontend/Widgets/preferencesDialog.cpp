#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"
#include "logger.h"

#include "caseInformationModel.h"
#include "Utility/userSettings.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    m_physicianLabels = {ui->labelDr1, ui->labelDr2, ui->labelDr3};
    m_locationLabels = {ui->labelLocation1, ui->labelLocation2, ui->labelLocation3, };

    setWindowFlags(Qt::SplashScreen);

    connect(ui->labelDr1, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedPhysician);
    connect(ui->labelDr2, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedPhysician);
    connect(ui->labelDr3, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedPhysician);

    connect(ui->labelLocation1, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedLocation);
    connect(ui->labelLocation2, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedLocation);
    connect(ui->labelLocation3, &ActiveLabel::labelSelected, this, &PreferencesDialog::handleSelectedLocation);

    connect(ui->pushButtonDrDefault, &QPushButton::clicked, this, &PreferencesDialog::setDefaultPhysician);
    connect(ui->pushButtonLocationDefault, &QPushButton::clicked, this, &PreferencesDialog::setDefaultLocation);

    connect(ui->pushButtonDone, &QPushButton::clicked, this, &PreferencesDialog::persistDefaults);

    initPhysiciansContainer();
    initLocationsContainer();

    const auto& ci = CaseInformationModel::instance();
    m_defaultLocationCandidate = ci->defaultLocation();
    m_defaultPhysicianCandidate = ci->defaultPhysicianName();

    setDefaultPhysician();
    setDefaultLocation();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::handleSelectedPhysician(const QString &name)
{
    LOG1(name)
    for(auto& label : m_physicianLabels){
        label->setStyleSheet("color: white");
    }

    const auto& labelIt = m_physiciansContainer.find(name);
    if(labelIt != m_physiciansContainer.end()){
        m_selectedPhysicianLabel = labelIt->second;
        if(m_selectedPhysicianLabel){
            m_selectedPhysicianLabel->setStyleSheet("color: rgb(245,196,0)");
        }
    }
    m_defaultPhysicianCandidate = name;
    ui->pushButtonDrDefault->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
}

void PreferencesDialog::setDefaultPhysician()
{
    ui->labelDrDefault->setText(QString("Default: ") + m_defaultPhysicianCandidate);
    ui->pushButtonDrDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
    if(m_selectedPhysicianLabel){
        m_selectedPhysicianLabel->setStyleSheet("color: white");
    }
    auto ci = CaseInformationModel::instance();
    ci->setDefaultPhysicianName(m_defaultPhysicianCandidate);
}

void PreferencesDialog::handleSelectedLocation(const QString &name)
{
    LOG1(name)
    for(auto& label : m_locationLabels){
        label->setStyleSheet("color: white");
    }

    const auto& labelIt = m_locationsContainer.find(name);
    if(labelIt != m_locationsContainer.end()){
        m_selectedLocationLabel = labelIt->second;
        if(m_selectedLocationLabel){
            m_selectedLocationLabel->setStyleSheet("color: rgb(245,196,0)");
        }
    }
    m_defaultLocationCandidate = name;
    ui->pushButtonLocationDefault->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
}

void PreferencesDialog::setDefaultLocation()
{
    ui->labelLocationDefault->setText(QString("Default: ") + m_defaultLocationCandidate);
    ui->pushButtonLocationDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
    if(m_selectedLocationLabel){
        m_selectedLocationLabel->setStyleSheet("color: white");
    }
    auto ci = CaseInformationModel::instance();
    ci->setDefaultLocation(m_defaultLocationCandidate);
}

void PreferencesDialog::initPhysiciansContainer()
{
    const auto& ci = CaseInformationModel::instance();
    const auto& phns = ci->physicianNames();

    int i{0};
    for(const auto& label : m_physicianLabels){
        if(i < phns.size()){
            label->setText(phns[i]);
        }
        m_physiciansContainer[label->text()] = label;
        ++i;
    }
}

void PreferencesDialog::initLocationsContainer()
{
    const auto& ci = CaseInformationModel::instance();
    const auto& locs = ci->locations();

    int i{0};
    for(const auto& label : m_locationLabels){
        if(i < locs.size()){
            label->setText(locs[i]);
        }
        m_locationsContainer[label->text()] = label;
        ++i;
    }
}

void PreferencesDialog::persistDefaults()
{
    auto& settings = userSettings::Instance();
    const auto& ci = CaseInformationModel::instance();

    const auto& loc = ci->defaultLocation();
    LOG1(loc);
    settings.setLocation(loc);
    settings.setPhysician(ci->defaultPhysicianName());
}
