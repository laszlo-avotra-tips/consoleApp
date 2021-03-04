#include "caseInformationDialog.h"
#include "ui_caseInformationDialog.h"
#include "Utility/widgetcontainer.h"
#include "consoleLineEdit.h"
#include "selectDialog.h"
#include "caseInformationModel.h"
#include "logger.h"
#include "displayManager.h"
#include "defaults.h"

#include <QDateTime>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QShowEvent>
#include <algorithm>


CaseInformationDialog::CaseInformationDialog(QWidget *parent, const std::vector<QString> *param) :
    QDialog(parent),
    ui(new Ui::CaseInformationDialog),
    m_model(*CaseInformationModel::instance())
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    /*
     * connect the user interface to this class
     */
    connect(ui->lineEditPhysicianName, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::editOrSelectPhysicianName);
    connect(ui->lineEditPatientId, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardPatientId);
    connect(ui->lineEditLocation, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::editOrSelectLocation);

    connect(ui->pushButtonBack, &QPushButton::clicked, this, &CaseInformationDialog::handleBack);
    connect(ui->pushButtonNext, &QPushButton::clicked, this, &CaseInformationDialog::handleNext);
    connect(ui->pushButtonPhysicianNameDown, &QPushButton::clicked, this, &CaseInformationDialog::handlePhysicianNameSelect);
    connect(ui->pushButtonLocationDown, &QPushButton::clicked, this, &CaseInformationDialog::handleLocationSelect);

    initDialog(param);

    const int xc = ControlScreenWidth / 2 - width() / 2;
    const int yc = ControlScreenHeight / 2 - height() / 2;
    move(xc,yc);

    const auto& ci = CaseInformationModel::instance();
    if(!ci->defaultPhysicianName().isEmpty()){
        ui->lineEditPhysicianName->setStyleSheet("");
        ui->lineEditPhysicianName->setText(ci->defaultPhysicianName());
        enableNext(true);
    }
    if(!ci->defaultLocation().isEmpty()){
        ui->lineEditLocation->setText(ci->defaultLocation());
    }
}

CaseInformationDialog::~CaseInformationDialog()
{
    m_displayTimer.stop();
    delete ui;
}

void CaseInformationDialog::reset()
{
    /*
     * remove selections
     */
    CaseInformationModel::instance()->setSelectedPhysicianName("");
    CaseInformationModel::instance()->setSelectedLocation("");
}

void CaseInformationDialog::initDialog(const std::vector<QString> *param)
{
    if(param && !param->empty()){
        const auto& enterButtonText = *param->begin();
        ui->pushButtonNext->setText(enterButtonText);
        m_isNewCase = false;
    }

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

    /*
     * required field logic
     */
    if(m_model.isSelectedPhysicianName()){
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
        ui->lineEditDateAndTime->setText(m_model.dateAndTime());
        enableNext(true);
        if(!m_model.patientId().isEmpty()){
            ui->lineEditPatientId->setText(m_model.patientId());
        }
        if(!m_model.dateAndTime().isEmpty()){
            ui->lineEditDateAndTime->setText(m_model.dateAndTime());
        }
        if(!m_model.selectedLocation().isEmpty()){
            ui->lineEditLocation->setText(m_model.selectedLocation());
        }
    } else {
        setDateAndTime();
        connect(&m_displayTimer, &QTimer::timeout, this, &CaseInformationDialog::setDateAndTime);
        m_displayTimer.start(500);
        enableNext(false);
        m_model.setSelectedLocation("");
        m_model.setPatientId("");
    }
}

int CaseInformationDialog::indexOf(const PhysicianNameContainer &cont, const QString &val) const
{
    const auto it = std::find(cont.begin(), cont.end(), val);

    auto base = cont.begin();

    int index{0};
    while( base != it){
        ++base;
        ++index;
    }

    //int index = it - cont.begin();

    return index;
}

void CaseInformationDialog::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    if(se->type() == QEvent::Show){
        DisplayManager::instance()->initWidgetForTheSecondMonitor("disk");
        DisplayManager::instance()->setWindowTitle("CASE INFORMATION IN PROCESS");
        WidgetContainer::instance()->setIsNewCase(true);
    }
}

void CaseInformationDialog::handleBack()
{
    if(m_isNewCase){
        m_model.setSelectedPhysicianName("");
        m_model.setSelectedLocation("");
        m_model.setPatientId("");
    }
    reject();
}

void CaseInformationDialog::setDateAndTime()
{
    /*
     * set the data and time in the Dialog
     */
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}

void CaseInformationDialog::editOrSelectPhysicianName()
{
    if(m_model.isSelectedPhysicianName()){
        /*
         * edit the physician name
         * initialize the keyboard for edit
         */
        const auto& list = m_model.physicianNames();
        const auto& value = m_model.selectedPhysicianName();
        int index = indexOf(list,value);
        QString paramName = ui->labelPhysicianName->text();
        QString paramValue = ui->lineEditPhysicianName->text();
        const int keyboardY{410};

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
        ui->lineEditPhysicianName->setText(newName);
        m_model.setSelectedPhysicianName(newName);
        m_model.setPhysicianName(index,newName);

        const bool isNext(!ui->lineEditPhysicianName->text().isEmpty());
        /*
         * enable disable Next
         */
        enableNext(isNext);
    } else {
        /*
         * select physicianName
         */
        emit ui->pushButtonPhysicianNameDown->clicked();
    }
}

void CaseInformationDialog::openKeyboardPatientId()
{
    /*
     * initialize the keyboard to create or edit patiant id
     */
    QString paramName = ui->labelPatientId->text();
    QString paramValue = ui->lineEditPatientId->text();
    const int keyboardY{510};

    /*
     * create the keyboard parameters
     */
    const std::vector<QString> param{paramName, paramValue, "ENTER"};

    /*
     * create the modal keyboard instance for patient ID
     */
    auto newPatientId = WidgetContainer::instance()->openKeyboard(this, param, keyboardY);


    /*
     * update patient ID with newPatientId
     */
    ui->lineEditPatientId->setText(newPatientId);
    m_model.setPatientId(newPatientId);
}

void CaseInformationDialog::editOrSelectLocation()
{
    if(m_model.isSelectedLocation()){
        /*
         * edit the location
         * initialize the keyboard for edit
         */
        const auto& list = m_model.locations();
        const auto& value = m_model.selectedLocation();
        int index = indexOf(list,value);
        QString paramName = ui->labelLocation->text();
        QString paramValue = ui->lineEditLocation->text();
        const int keyboardY{610};

        /*
         * create the keyboard parameters
         */
        const std::vector<QString> param{paramName, paramValue};

        /*
         * create the modal keyboard instance for location
         */
        auto newLocation = WidgetContainer::instance()->openKeyboard(this, param, keyboardY);

        /*
         * code execution continues here once the keyboard is closed
         * update selected location with newLocation
         */
        ui->lineEditLocation->setText(newLocation);
        m_model.setSelectedLocation(newLocation);
        m_model.setLocation(index, newLocation);

    } else {
        /*
         * select location
         */
        emit ui->pushButtonLocationDown->clicked();
    }
}

void CaseInformationDialog::handleNext()
{
    m_model.setDateAndTime(ui->lineEditDateAndTime->text());

    /*
     * Create the session directory
     */
    m_model.validate();

    accept();
}

void CaseInformationDialog::enableNext(bool isNext)
{
    ui->pushButtonNext->setEnabled(isNext);

    /*
     * highlight the Next button if true
     */
    if(isNext){
        ui->frameNext->setStyleSheet("background-color:#F5C400;");
    } else {
        ui->frameNext->setStyleSheet("background-color:#262626;");
    }
}

void CaseInformationDialog::handlePhysicianNameSelect()
{
    auto* parent = this;

    /*
     * create the modal select dialog
     */
    m_selectDialog = new SelectDialog(parent);

    /*
     * move the select dialog
     */
    const int xVal = x() + parent->width()/2 - m_selectDialog->width()/2 + 300;
    const int yVal = y() + 430;

    m_selectDialog->move(xVal, yVal);
    m_selectDialog->show();

    /*
     * populate the select dialog with physician names
     */
    m_selectDialog->populate(m_model.physicianNames(), m_model.defaultPhysicianName());

    if(m_selectDialog->exec() == QDialog::Accepted){

        /*
         * handle selection was made
         */
        m_model.setSelectedPhysicianName(m_selectDialog->selectedItem());
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
    } else {

        /*
         * handle "ADD NEW" physician name
         */
        QString paramName = ui->labelPhysicianName->text();
        QString paramValue("");
        const int keyboardY{200};
        const ParameterType param{paramName, paramValue, "ADD NEW"};

        /*
         * create the modal keyboard instance for physician name
         */
        auto newName = WidgetContainer::instance()->openKeyboard(this, param, keyboardY);

        /*
         * code execution continues here once the keyboard is closed
         * add newName
         * update selected physician name with newName
         */
        m_model.addPhysicianName(newName);
        m_model.setSelectedPhysicianName(newName);
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());

        ui->lineEditPhysicianName->setStyleSheet("");
    }

    /*
     * update isNext condition
     */
    const bool isNext(!ui->lineEditPhysicianName->text().isEmpty());
    enableNext(isNext);
}

void CaseInformationDialog::handleLocationSelect()
{
    auto* parent = this;

    /*
     * create the modal select dialog
     */
    m_selectDialog = new SelectDialog(parent);

    /*
     * move the select dialog
     */
    const int xVal = x() + parent->width()/2 - m_selectDialog->width()/2 + 300;
    const int yVal = y() + 630;

    m_selectDialog->move(xVal, yVal);
    m_selectDialog->show();

    m_selectDialog->populate(m_model.locations(), m_model.defaultLocation());

    if(m_selectDialog->exec() == QDialog::Accepted){

        /*
         * handle selection was made
         */
        const auto& location =  m_selectDialog->selectedItem();
        ui->lineEditLocation->setText(location);
        m_model.setSelectedLocation(location);
    } else {
        /*
         * handle "ADD NEW" physician name
         */
        QString paramName = ui->labelLocation->text();
        QString paramValue("");
        const int keyboardY{200};
        const ParameterType param{paramName, paramValue, "ADD NEW"};

        /*
         * create the modal keyboard instance for location
         */
        auto newLocation = WidgetContainer::instance()->openKeyboard(this, param, keyboardY);


        /*
         * code execution continues here once the keyboard is closed
         * add newLocation
         * update selected location with newLocation
         */
        m_model.addLocation(newLocation);
        ui->lineEditLocation->setText(newLocation);

        m_model.setSelectedLocation(newLocation);
    }
}
