#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"
#include "logger.h"
#include "Utility/preferencesModel.h"

#include "Utility/userSettings.h"
#include "Utility/preferencesDatabase.h"
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

    connect(&m_currentDateUpdate, &QTimer::timeout, this, &PreferencesDialog::setCurrentDate);
    connect(&m_currentTimeUpdate, &QTimer::timeout, this, &PreferencesDialog::setCurrentTime);

    m_currentDateUpdate.start(1000);
    m_currentTimeUpdate.start(100);

    ui->pushButtonAddLocation->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
    ui->pushButtonAddPhysician->setStyleSheet("background-color: rgb(245,196,0); color: black; font: 18pt;");
}

void PreferencesDialog::init()
{
    m_model = PreferencesModel::instance();

    m_defaultLocationCandidate = m_model->defaultLocation();
    m_defaultPhysicianCandidate = m_model->defaultPhysician();
    LOG2(m_defaultPhysicianCandidate, m_defaultLocationCandidate);

    m_locIt = m_model->locations().begin();
    m_phIt = m_model->physicians().begin();

    initPhysiciansContainer();
    initLocationsContainer();

    setDefaultPhysician();
    setDefaultLocation();

    ui->labelSn->setText(QString("Lightbox Serial Number: ") + m_model->getSerialNumber());
    ui->labelDiskSpace->setText(QString("Disk Space Remaining: ") + m_model->getSpaceRemaining() + QString(" GB"));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::handleLabelDr1(ActiveLabel *label)
{
    handlePhysicianLabel(label);
}

void PreferencesDialog::handleLabelDr2(ActiveLabel* label)
{
    handlePhysicianLabel(label);
}

void PreferencesDialog::handleLabelDr3(ActiveLabel* label)
{
    handlePhysicianLabel(label);
}

void PreferencesDialog::handleLocation1(ActiveLabel *label)
{
    handleLocationLabel(label);
}

void PreferencesDialog::handleLocation2(ActiveLabel *label)
{
    handleLocationLabel(label);
}

void PreferencesDialog::handleLocation3(ActiveLabel *label)
{
    handleLocationLabel(label);
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
        } else {
            LOG1(m_selectedPhysicianLabel);
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
    m_model->setDefaultPhysician(m_defaultPhysicianCandidate);
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
        } else {
            LOG1(m_selectedLocationLabel);
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
    m_model->setDefaultLocation(m_defaultLocationCandidate);
    m_defaultLocationCandidate = "";
    ui->pushButtonAddLocation->setText("ADD");
}

void PreferencesDialog::initPhysiciansContainer()
{
    const auto& names = m_model->physicians();

    auto nameIt = m_phIt;
    m_physiciansContainer.erase(m_physiciansContainer.begin(), m_physiciansContainer.end());
    for(auto* label : m_physicianLabels){
        if(label && nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
            m_physiciansContainer[label->text()] = label;
            ++nameIt;
        } else if(label){
            label->setText("");
        }
    }
}

void PreferencesDialog::initLocationsContainer()
{
    const auto& names = m_model->locations();

    auto nameIt = m_locIt;
    m_locationsContainer.erase(m_locationsContainer.begin(), m_locationsContainer.end());
    for(auto* label : m_locationLabels){
        if(label && nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
            m_locationsContainer[label->text()] = label;
            ++nameIt;
        } else if(label){
            label->setText("");
        }
    }
}

void PreferencesDialog::updatePysicianLabels()
{
    const auto& names = m_model->physicians();
    m_phIt = names.begin();
    auto nameIt = m_phIt;

    m_physiciansContainer.erase(m_physiciansContainer.begin(), m_physiciansContainer.end());
    for(const auto& label : m_physicianLabels){
        if(nameIt != names.end()){
            const auto& name = *nameIt;
            LOG1(name);
            label->setText(name);
             m_physiciansContainer[name] = label;
             ++nameIt;
        } else {
            label->setText("");
        }
    }
}


void PreferencesDialog::updateLocationLabels()
{
    const auto& names = m_model->locations();

    m_locIt = names.begin();
    auto nameIt = m_locIt;

    m_locationsContainer.erase(m_locationsContainer.begin(), m_locationsContainer.end());
    for(const auto& label : m_locationLabels){
        if(nameIt != names.end()){
            const auto& name = *nameIt;
            label->setText(name);
             m_locationsContainer[name] = label;
             ++nameIt;
        }else {
            label->setText("");
        }
    }
}

void PreferencesDialog::persistPreferences()
{
    m_model->persistPreferences();
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
        m_model->addLocation(newName);
        updateLocationLabels();
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
        m_model->addPhysician(newName);
        updatePysicianLabels();
    }
}

void PreferencesDialog::handleRemoveLocation()
{
    if(m_selectedLocationLabel){
        const auto& name = m_selectedLocationLabel->text();
        LOG1(name);
        if(m_model->removeLocation(name)){
            updateLocationLabels();
            unmarkAll(m_locationLabels);
            ui->pushButtonAddLocation->setText("ADD");
            ui->pushButtonLocationDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
            if(m_model->defaultLocation() == name){
                ui->labelLocationDefault->setText("Default:");
                m_model->setDefaultLocation("");
            }
        }
    }
}

void PreferencesDialog::handleRemovePhysician()
{
    if(m_selectedPhysicianLabel){
        const auto& name = m_selectedPhysicianLabel->text();
        LOG1(name);

        if(m_model->removePhysician(name)){
            LOG1(name);
            updatePysicianLabels();
            unmarkAll(m_physicianLabels);
            ui->pushButtonAddPhysician->setText("ADD");
            ui->pushButtonDrDefault->setStyleSheet("background-color:#676767; color: black; font: 18pt;");
            if(m_model->defaultPhysician() == name){
                ui->labelDrDefault->setText("Default:");
                m_model->setDefaultPhysician("");
            }
        }
    }
}

void PreferencesDialog::handlePhysicianUp()
{
    const auto& names = m_model->physicians();

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
    const auto& names = m_model->physicians();

    if(m_phIt != names.begin()){
        --m_phIt;
        initPhysiciansContainer();
    }
    handlePhysicianUnmarked();
    unmarkAll(m_physicianLabels);
}

void PreferencesDialog::handleLocationUp()
{
    const auto& names = m_model->locations();

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
    const auto& names = m_model->locations();

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

void PreferencesDialog::setCurrentDate()
{
    ui->labelDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy")) ;
}

void PreferencesDialog::setCurrentTime()
{
    ui->labelTime->setText(QDateTime::currentDateTime().toString(" hh:mm:ss")) ;
}

void PreferencesDialog::handlePhysicianLabel(const ActiveLabel *label)
{
    const auto& name = label->text();
    if(!name.isEmpty()){
        handleSelectedPhysician(name);
    }
}

void PreferencesDialog::handleLocationLabel(const ActiveLabel *label)
{
    const auto& name = label->text();
    if(!name.isEmpty()){
        handleSelectedLocation(name);
    }
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
