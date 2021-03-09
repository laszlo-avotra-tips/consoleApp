#include "caseInformationDialog.h"
#include "ui_caseInformationDialog.h"
#include "Utility/widgetcontainer.h"
#include "consoleLineEdit.h"
#include "selectDialog.h"
#include "caseInformationModel.h"
#include "logger.h"
#include "displayManager.h"
#include "defaults.h"
#include "Utility/preferencesModel.h"

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
    connect(ui->lineEditPatientId, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardPatientId);

    connect(ui->pushButtonBack, &QPushButton::clicked, this, &CaseInformationDialog::handleBack);
    connect(ui->pushButtonNext, &QPushButton::clicked, this, &CaseInformationDialog::handleNext);
    connect(ui->pushButtonPhysicianNameDown, &QPushButton::toggled, this, &CaseInformationDialog::handlePhysicianNameSelect);
    connect(ui->pushButtonLocationDown, &QPushButton::clicked, this, &CaseInformationDialog::handleLocationSelect);

    initDialog(param);

    const int xc = ControlScreenWidth / 2 - width() / 2;
    const int yc = ControlScreenHeight / 2 - height() / 2;
    move(xc,yc);

}

CaseInformationDialog::~CaseInformationDialog()
{
    m_displayTimer.stop();
    delete ui;
}

void CaseInformationDialog::initDialog(const std::vector<QString> *param)
{
    if(param && !param->empty()){
        const auto& enterButtonText = *param->begin();
        ui->pushButtonNext->setText(enterButtonText);
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
        LOG1(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
        ui->lineEditDateAndTime->setText(m_model.dateAndTime());
        enableNext();
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
        ui->lineEditPhysicianName->setText("Required field");
        enableNext();
        m_model.setSelectedLocation("");
        m_model.setPatientId("");
    }
    setDateAndTime();
    connect(&m_displayTimer, &QTimer::timeout, this, &CaseInformationDialog::setDateAndTime);
    m_displayTimer.start(500);
}

QString CaseInformationDialog::getPhysicianName() const
{
    return ui->labelPhysicianName->text();
}

void CaseInformationDialog::setPhysicianName(const QString &name)
{
    ui->lineEditPhysicianName->setText(name);
    ui->lineEditPhysicianName->setStyleSheet("");
    enableNext();
}

QString CaseInformationDialog::getLocation() const
{
    return ui->labelLocation->text();
}

void CaseInformationDialog::setLocation(const QString &name)
{
    ui->lineEditLocation->setText(name);
    ui->lineEditLocation->setStyleSheet("");
}

void CaseInformationDialog::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    if(se->type() == QEvent::Show){
        DisplayManager::instance()->initWidgetForTheSecondMonitor("disk");
        DisplayManager::instance()->setWindowTitle("CASE INFORMATION IN PROCESS");
        WidgetContainer::instance()->setIsNewCase(true);
        if(m_model.selectedPhysicianName().isEmpty()){
            ui->lineEditPhysicianName->setText("Required field");
            ui->lineEditPhysicianName->setStyleSheet("color:grey;font:12pt \"Helvetica Neue\"");
        }else{
            ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        }
        ui->lineEditLocation->setText(m_model.selectedLocation());
    }
}

void CaseInformationDialog::handleBack()
{
    reject();
}

void CaseInformationDialog::setDateAndTime()
{
    /*
     * set the data and time in the Dialog
     */
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
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

void CaseInformationDialog::handleNext()
{
    m_model.setDateAndTime(ui->lineEditDateAndTime->text());

    /*
     * Create the session directory
     */
    m_model.validate();

    accept();
}

void CaseInformationDialog::enableNext()
{
    bool isNext = false;
    //Required field
    if(!ui->lineEditPhysicianName->text().isEmpty() && ui->lineEditPhysicianName->text() != "Required field"){
        isNext = true;
    }
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

void CaseInformationDialog::handlePhysicianNameSelect(bool isChecked)
{
    LOG2(isChecked,m_selectDialog);
    if(isChecked)
    {
        if(m_selectDialog){
            m_selectDialog->reject();
        }
        return;
    }

    /*
     * create the modal select dialog
     */
    m_selectDialog = new SelectDialog("physician", this);
    m_selectDialog->setModal(false);
    ui->lineEditPhysicianName->setEnabled(false);

    /*
     * move the select dialog
     */
    const int xVal = x() + width()/2 - m_selectDialog->width()/2 + 305;
    const int yVal = y() + 430;

    m_selectDialog->move(xVal, yVal);
    m_selectDialog->show();

    /*
     * populate the select dialog with physician names
     */
    LOG1(m_model.selectedPhysicianName());
    m_selectDialog->initializeSelect(PreferencesModel::instance()->physicians(), m_model.selectedPhysicianName());
    if(m_selectDialog->exec() == QDialog::Accepted){
        /*
         * handle selection was made
         */
        LOG1(m_selectDialog->selectedItem());
        m_model.setSelectedPhysicianName(m_selectDialog->selectedItem());
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
        ui->pushButtonPhysicianNameDown->toggle();
    } else {
        LOG1(isChecked);
        if(!isChecked){
            ui->lineEditPhysicianName->setEnabled(true);
            return;
        }
    }

    /*
     * update isNext condition
     */
    ui->lineEditPhysicianName->setEnabled(true);
    enableNext();
}

void CaseInformationDialog::handleLocationSelect(bool isChecked)
{
    LOG2(isChecked,m_selectDialog);
    if(isChecked)
    {
        if(m_selectDialog){
            m_selectDialog->reject();
        }
        return;
    }
    auto* cid = this;

    /*
     * create the modal select dialog
     */
    m_selectDialog = new SelectDialog("location",cid);
    m_selectDialog->setModal(false);

    ui->lineEditLocation->setEnabled(false);
    ui->lineEditPhysicianName->setEnabled(false);
    ui->lineEditPatientId->setEnabled(false);
    ui->pushButtonPhysicianNameDown->setEnabled(false);
    /*
     * move the select dialog
     */
    const int xVal = x() + cid->width()/2 - m_selectDialog->width()/2 + 305;
    const int yVal = y() + 630;

    m_selectDialog->move(xVal, yVal);
    m_selectDialog->show();

    /*
     * populate the select dialog with physician names
     */
    LOG1(m_model.selectedLocation());
    m_selectDialog->initializeSelect(PreferencesModel::instance()->locations(), m_model.selectedLocation());

    if(m_selectDialog->exec() == QDialog::Accepted){

        /*
         * handle selection was made
         */
        LOG1(m_selectDialog->selectedItem());
        m_model.setSelectedLocation(m_selectDialog->selectedItem());
        ui->lineEditLocation->setText(m_model.selectedLocation());
        ui->pushButtonLocationDown->toggle();
    }
    else
    {
        LOG1(isChecked);
        if(!isChecked){
            ui->lineEditLocation->setEnabled(true);
            ui->lineEditPhysicianName->setEnabled(true);
            ui->lineEditPatientId->setEnabled(true);
            ui->pushButtonPhysicianNameDown->setEnabled(true);
            return;
        }
    }

    ui->lineEditLocation->setEnabled(true);
    ui->lineEditPhysicianName->setEnabled(true);
    ui->lineEditPatientId->setEnabled(true);
    ui->pushButtonPhysicianNameDown->setEnabled(true);

}
