#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"
#include "logger.h"

#include "caseInformationModel.h"
#include "Utility/userSettings.h"
#include "Utility/caseInfoDatabase.h"
#include "Utility/widgetcontainer.h"


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

    connect(ui->pushButtonDone, &QPushButton::clicked, this, &PreferencesDialog::persistPreferences);

    connect(ui->pushButtonAddLocation, &QPushButton::clicked, this, &PreferencesDialog::handleAddRemoveLocation);
    connect(ui->pushButtonAddPhysician, &QPushButton::clicked, this, &PreferencesDialog::handleAddRemovePhysician);

    ui->pushButtonAddLocation->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddPhysician->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");


    CaseInfoDatabase ciDb;
    ciDb.initDb();

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
    ui->pushButtonAddPhysician->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddPhysician->setText("REMOVE");
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
    ui->pushButtonAddPhysician->setText("+ADD");
//    ui->spushButtonAddPhysician->setStyleSheet("background-color:#262626; color: black; font: 18pt;");
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

    ui->pushButtonAddLocation->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddLocation->setText("REMOVE");
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
    ui->pushButtonAddLocation->setText("+ADD");
//    ui->pushButtonAddLocation->setStyleSheet("background-color:#262626; color: black; font: 18pt;");
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

void PreferencesDialog::initPhysiciansContainer2()
{
    const auto& ci = CaseInformationModel::instance();
    const auto& phns = ci->physicianNames2();

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

void PreferencesDialog::initLocationsContainer2()
{
    const auto& ci = CaseInformationModel::instance();
    const auto& locs = ci->locations2();

    int i{0};
    for(const auto& label : m_locationLabels){
        if(i < locs.size()){
            label->setText(locs[i]);
        }
        m_locationsContainer[label->text()] = label;
        ++i;
    }
}

void PreferencesDialog::createCaseInfoDb()
{
    initLocationsContainer2();
    initPhysiciansContainer2();
    //lcv only once
    CaseInfoDatabase ciDb;
    ciDb.initDb();
    for(const auto& physician : m_physiciansContainer){
        ciDb.addPhysician(physician.first);
    }

    for(const auto& location : m_locationsContainer){
        ciDb.addLocation(location.first);
    }
}

void PreferencesDialog::persistPreferences()
{
    auto& settings = userSettings::Instance();
    const auto& ci = CaseInformationModel::instance();

    const auto& loc = ci->defaultLocation();
    LOG1(loc);
    settings.setLocation(loc);
    settings.setPhysician(ci->defaultPhysicianName());

    //    createCaseInfoDb();
}

void PreferencesDialog::handleAddRemoveLocation()
{
    if(ui->pushButtonAddLocation->text() == "+ADD"){
        handleAddLocation();
    }else{
        handleRemoveLocation();
    }
}

void PreferencesDialog::handleAddRemovePhysician()
{
    if(ui->pushButtonAddPhysician->text() == "+ADD"){
        handleAddPhysician();
    }else{
        handleRemovePhysician();
    }

}

void PreferencesDialog::handleAddLocation()
{
//    ui->pushButtonAddLocation->setStyleSheet("background-color:#262626; color: black; font: 18pt;");
    LOG1(m_selectedLocationLabel->text())
}

void PreferencesDialog::handleAddPhysician()
{
//    ui->pushButtonAddPhysician->setStyleSheet("background-color:#262626; color: black; font: 18pt;");
    LOG1(m_selectedPhysicianLabel->text());
    QString paramName = "PHYSICIAN NAME";
    QString paramValue = m_selectedPhysicianLabel->text();
    const int keyboardY{-100};

    /*
     * create the keyboard parameters
     */
    const std::vector<QString> param{paramName, paramValue};

    /*
     * create the modal keyboard instance for physician name
     */
    auto newName = WidgetContainer::instance()->openKeyboard(this, param, keyboardY);

    /*
     * code execution continues here once the keyboard is closed
     * update selected physician name with newName
     */
    LOG1(newName);

}

void PreferencesDialog::handleRemoveLocation()
{
    LOG1(m_selectedLocationLabel->text())
}

void PreferencesDialog::handleRemovePhysician()
{
    LOG1(m_selectedPhysicianLabel->text())
}
