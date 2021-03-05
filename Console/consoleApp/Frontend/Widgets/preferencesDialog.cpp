#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"
#include "logger.h"

#include "caseInformationModel.h"
#include "Utility/userSettings.h"
#include "Utility/caseInfoDatabase.h"
#include "Utility/widgetcontainer.h"
#include "consoleKeyboard.h"
#include <QShowEvent>
#include <QHideEvent>


PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    m_physicianLabels = {ui->labelDr1, ui->labelDr2, ui->labelDr3};
    m_locationLabels = {ui->labelLocation1, ui->labelLocation2, ui->labelLocation3, };

    setWindowFlags(Qt::SplashScreen);

    connect(ui->labelDr1, &ActiveLabel::labelItemMarked, this, &PreferencesDialog::handleLabelDr1);
    connect(ui->labelDr2, &ActiveLabel::labelItemMarked, this, &PreferencesDialog::handleLabelDr2);
    connect(ui->labelDr3, &ActiveLabel::labelItemMarked, this, &PreferencesDialog::handleLabelDr3);

    connect(ui->labelDr1, &ActiveLabel::labelItemUnmarked, this, &PreferencesDialog::handlePhysicianUnmarked);
    connect(ui->labelDr2, &ActiveLabel::labelItemUnmarked, this, &PreferencesDialog::handlePhysicianUnmarked);
    connect(ui->labelDr3, &ActiveLabel::labelItemUnmarked, this, &PreferencesDialog::handlePhysicianUnmarked);

    connect(ui->labelLocation1, &ActiveLabel::labelItemMarked, this, &PreferencesDialog::handleLocation1);
    connect(ui->labelLocation2, &ActiveLabel::labelItemMarked, this, &PreferencesDialog::handleLocation2);
    connect(ui->labelLocation3, &ActiveLabel::labelItemMarked, this, &PreferencesDialog::handleLocation3);

    connect(ui->labelLocation1, &ActiveLabel::labelItemUnmarked, this, &PreferencesDialog::handleLocationUnmarked);
    connect(ui->labelLocation2, &ActiveLabel::labelItemUnmarked, this, &PreferencesDialog::handleLocationUnmarked);
    connect(ui->labelLocation3, &ActiveLabel::labelItemUnmarked, this, &PreferencesDialog::handleLocationUnmarked);

    connect(ui->pushButtonDrDefault, &QPushButton::clicked, this, &PreferencesDialog::setDefaultPhysician);
    connect(ui->pushButtonLocationDefault, &QPushButton::clicked, this, &PreferencesDialog::setDefaultLocation);

    connect(ui->pushButtonDone, &QPushButton::clicked, this, &PreferencesDialog::persistPreferences);

    connect(ui->pushButtonAddLocation, &QPushButton::clicked, this, &PreferencesDialog::handleAddRemoveLocation);
    connect(ui->pushButtonAddPhysician, &QPushButton::clicked, this, &PreferencesDialog::handleAddRemovePhysician);

    connect(ui->pushButtonDrUp, &QPushButton::clicked, this, &PreferencesDialog::handlePhysicianDown);
    connect(ui->pushButtonDrDown, &QPushButton::clicked, this, &PreferencesDialog::handlePhysicianUp);

    connect(ui->pushButtonLocationUp, &QPushButton::clicked, this, &PreferencesDialog::handleLocationDown);
    connect(ui->pushButtonLocationDown, &QPushButton::clicked, this, &PreferencesDialog::handleLocationUp);

    ui->pushButtonAddLocation->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddPhysician->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");

}

void PreferencesDialog::init()
{
    CaseInfoDatabase ciDb;
    ciDb.initCaseInfo();

    const auto& ci = CaseInformationModel::instance();
    m_defaultLocationCandidate = ci->defaultLocation();
    m_defaultPhysicianCandidate = ci->defaultPhysicianName();

    m_locIt = ci->locations().begin();
    m_phIt = ci->physicianNames().begin();
    initPhysiciansContainer();
    initLocationsContainer();

    setDefaultPhysician();
    setDefaultLocation();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::handleLabelDr1(ActiveLabel *label)
{
    const auto& name = label->text();
    handleSelectedPhysician(name);
}

void PreferencesDialog::handleLabelDr2(ActiveLabel* label)
{
    const auto& name = label->text();
    handleSelectedPhysician(name);
}

void PreferencesDialog::handleLabelDr3(ActiveLabel* label)
{
    const auto& name = label->text();
    handleSelectedPhysician(name);
}

void PreferencesDialog::handleLocation1(ActiveLabel *label)
{
    const auto& name = label->text();
    handleSelectedLocation(name);
}

void PreferencesDialog::handleLocation2(ActiveLabel *label)
{
    const auto& name = label->text();
    handleSelectedLocation(name);
}

void PreferencesDialog::handleLocation3(ActiveLabel *label)
{
    const auto& name = label->text();
    handleSelectedLocation(name);
}

void PreferencesDialog::unmarkAll(std::vector<ActiveLabel*>& container)
{
    for(auto& label : container){
        label->unmark();
    }
}

void PreferencesDialog::markCandidate(std::vector<ActiveLabel *> &cont, const QString &name)
{
    // for empty default - "" - all will be unmarked
    for(auto& label : cont){
        if(label->text() == name){
            label->mark();
        } else {
            label->unmark();
        }
    }
}


void PreferencesDialog::handleSelectedPhysician(const QString &name)
{
    unmarkAll(m_physicianLabels);

    const auto& labelIt = m_physiciansContainer.find(name);
    if(labelIt != m_physiciansContainer.end()){
        m_selectedPhysicianLabel = labelIt->second;
        if(m_selectedPhysicianLabel){
            m_selectedPhysicianLabel->mark();
        }
    }
    m_defaultPhysicianCandidate = name;
    ui->pushButtonDrDefault->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");

    ui->pushButtonAddPhysician->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddPhysician->setText("REMOVE");
    LOG2(m_defaultPhysicianCandidate, m_defaultLocationCandidate);
}

void PreferencesDialog::setDefaultPhysician()
{
    ui->labelDrDefault->setText(QString("Default: ") + m_defaultPhysicianCandidate);
    ui->pushButtonDrDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
    if(m_selectedPhysicianLabel){
        m_selectedPhysicianLabel->unmark();
    }
    auto ci = CaseInformationModel::instance();
    ci->setDefaultPhysicianName(m_defaultPhysicianCandidate);
    m_defaultPhysicianCandidate = "";
    ui->pushButtonAddPhysician->setText("ADD");
}

void PreferencesDialog::handleSelectedLocation(const QString &name)
{
    unmarkAll(m_locationLabels);

    const auto& labelIt = m_locationsContainer.find(name);
    if(labelIt != m_locationsContainer.end()){
        m_selectedLocationLabel = labelIt->second;
        if(m_selectedLocationLabel){
            m_selectedLocationLabel->mark();
        }
    }
    m_defaultLocationCandidate = name;
    ui->pushButtonLocationDefault->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");

    ui->pushButtonAddLocation->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddLocation->setText("REMOVE");
    LOG2(m_defaultPhysicianCandidate, m_defaultLocationCandidate);
}

void PreferencesDialog::setDefaultLocation()
{
    ui->labelLocationDefault->setText(QString("Default: ") + m_defaultLocationCandidate);
    ui->pushButtonLocationDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
    if(m_selectedLocationLabel){
        m_selectedLocationLabel->unmark();
    }
    auto ci = CaseInformationModel::instance();
    ci->setDefaultLocation(m_defaultLocationCandidate);
    m_defaultLocationCandidate = "";
    ui->pushButtonAddLocation->setText("ADD");
}

void PreferencesDialog::initPhysiciansContainer()
{
    const auto& ci = CaseInformationModel::instance();
    const auto& names = ci->physicianNames();

    auto nameIt = m_phIt;
    m_physiciansContainer.erase(m_physiciansContainer.begin(), m_physiciansContainer.end());
    for(const auto& label : m_physicianLabels){
        if(nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
            m_physiciansContainer[label->text()] = label;
            ++nameIt;
        }
    }
}

void PreferencesDialog::initLocationsContainer()
{
    const auto& ci = CaseInformationModel::instance();
    const auto& names = ci->locations();

    auto nameIt = m_locIt;
    m_locationsContainer.erase(m_locationsContainer.begin(), m_locationsContainer.end());
    for(const auto& label : m_locationLabels){
        if(nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
            m_locationsContainer[label->text()] = label;
            ++nameIt;
        }
    }
}

void PreferencesDialog::updatePysicianLabels(const QString& name)
{
    const auto& ci = CaseInformationModel::instance();
    const auto& names = ci->physicianNames();

    m_phIt = names.begin();
//    if(!name.isEmpty()){
//        auto temp = names.find(name);
//        if(temp != names.end()){
//            m_phIt = temp;
//        }
//    }
    auto nameIt = m_phIt;

    for(const auto& label : m_physicianLabels){
        if(nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
             m_physiciansContainer[name] = label;
             ++nameIt;
        }
    }
}

void PreferencesDialog::updateLocationLabels(const QString& name)
{
    const auto& ci = CaseInformationModel::instance();
    const auto& names = ci->locations();

    m_locIt = names.begin();
//    if(!name.isEmpty()){
//        auto temp = names.find(name);
//        if(temp != names.end()){
//            m_locIt = temp;
//        }
//    }
    auto nameIt = names.begin();

    for(const auto& label : m_locationLabels){
        if(nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
             m_locationsContainer[name] = label;
             ++nameIt;
        }
    }
}

void PreferencesDialog::persistPreferences()
{
    auto& settings = userSettings::Instance();
    const auto& ci = CaseInformationModel::instance();

    const auto& loc = ci->defaultLocation();
    const auto& pn = ci->defaultPhysicianName();

    settings.setLocation(loc);
    settings.setPhysician(pn);

    CaseInfoDatabase ciDb;
    ciDb.initCaseInfo();

    QSqlQuery q;

    q.prepare( QString("DELETE FROM Physicians"));
    q.exec();
    QSqlError sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }

    q.prepare( QString("DELETE FROM Locations"));
    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }

    for(const auto& physician : ci->physicianNames()){
        ciDb.addPhysician(physician);
    }

    for(const auto& location : ci->locations()){
        ciDb.addLocation(location);
    }
}

void PreferencesDialog::handleAddRemoveLocation()
{
    if(ui->pushButtonAddLocation->text() == "ADD"){
        handleAddLocation();
    }else{
        handleRemoveLocation();
    }
}

void PreferencesDialog::handleAddRemovePhysician()
{
    if(ui->pushButtonAddPhysician->text() == "ADD"){
        handleAddPhysician();
    }else{
        handleRemovePhysician();
    }

}

void PreferencesDialog::handleAddLocation()
{
    QString paramName("LOCATION");
    QString paramValue("");
    const int keyboardY{height() / 2};

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

    if(!newName.isEmpty()){
        auto cim = CaseInformationModel::instance();
        cim->addLocation(newName);
        updateLocationLabels(newName);
    }

}

void PreferencesDialog::handleAddPhysician()
{
    QString paramName("PHYSICIAN NAME");
    QString paramValue("");
    const int keyboardY{height() / 2};

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

    if(!newName.isEmpty()){
        auto cim = CaseInformationModel::instance();
        cim->addPhysicianName(newName);
        updatePysicianLabels(newName);
    }
}

void PreferencesDialog::handleRemoveLocation()
{
    const auto& name = m_selectedLocationLabel->text();

    auto cim = CaseInformationModel::instance();
    if(cim->removeLocation(name)){
        updateLocationLabels("");
        unmarkAll(m_locationLabels);
        ui->pushButtonAddLocation->setText("ADD");
        ui->pushButtonLocationDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
        if(cim->defaultLocation() == name){
            ui->labelLocationDefault->setText("Default:");
        }
    }
}

void PreferencesDialog::handleRemovePhysician()
{
    const auto& name = m_selectedPhysicianLabel->text();

    auto cim = CaseInformationModel::instance();
    if(cim->removePhysicianName(name)){
        updatePysicianLabels("");
        unmarkAll(m_physicianLabels);
        ui->pushButtonAddPhysician->setText("ADD");
        ui->pushButtonDrDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
        if(cim->defaultPhysicianName() == name){
            ui->labelDrDefault->setText("Default:");
        }
    }
}

void PreferencesDialog::handlePhysicianUp()
{
    auto cim = CaseInformationModel::instance();
    const auto& names = cim->physicianNames();

    auto maxBaseIt = names.end();
    int i=0;
    while((i < 3) && (maxBaseIt != names.begin())){
        --maxBaseIt;
        ++i;
    }
    if(m_phIt != maxBaseIt){
        ++m_phIt;
        initPhysiciansContainer();
    }
    handlePhysicianUnmarked();
    unmarkAll(m_physicianLabels);
}

void PreferencesDialog::handlePhysicianDown()
{
    auto cim = CaseInformationModel::instance();
    const auto& names = cim->physicianNames();

    if(m_phIt != names.begin()){
        --m_phIt;
        initPhysiciansContainer();
    }
    handlePhysicianUnmarked();
    unmarkAll(m_physicianLabels);
}

void PreferencesDialog::handleLocationUp()
{
    auto cim = CaseInformationModel::instance();
    const auto& names = cim->locations();

    auto maxBaseIt = names.end();
    int i=0;
    while((i < 3) && (maxBaseIt != names.begin())){
        --maxBaseIt;
        ++i;
    }
    if(m_locIt != maxBaseIt){
        ++m_locIt;
        initLocationsContainer();
    }
    handleLocationUnmarked();
    unmarkAll(m_locationLabels);
}

void PreferencesDialog::handleLocationDown()
{
    auto cim = CaseInformationModel::instance();
    const auto& names = cim->locations();

    if(m_locIt != names.begin()){
        --m_locIt;
        initLocationsContainer();
    }
    handleLocationUnmarked();
    unmarkAll(m_locationLabels);
}

void PreferencesDialog::handlePhysicianUnmarked()
{
    ui->pushButtonAddPhysician->setText("ADD");
    ui->pushButtonDrDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
    m_defaultPhysicianCandidate = "";
}

void PreferencesDialog::handleLocationUnmarked()
{
    ui->pushButtonAddLocation->setText("ADD");
    ui->pushButtonLocationDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
    m_defaultLocationCandidate = "";
}

void PreferencesDialog::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    if(se->type() == QEvent::Show){
        LOG1(se->type());
        init();
    }
}

void PreferencesDialog::hideEvent(QHideEvent *he)
{
    QWidget::hideEvent( he );
    if(he->type() == QEvent::Hide){
        LOG1(he->type());
    }
}
