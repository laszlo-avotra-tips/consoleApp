#include "caseInformationDialog.h"
#include "ui_caseInformationDialog.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "consoleLineEdit.h"
#include "selectDialog.h"

#include <QDateTime>
#include <QTimer>

CaseInformationModel CaseInformationDialog::m_model;

CaseInformationDialog::CaseInformationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseInformationDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    connect(ui->lineEditPhysicianName, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardPhysicianName);
    connect(ui->lineEditPatientId, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardPatientId);
    connect(ui->lineEditLocation, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardLocation);

    QTimer::singleShot(100, this,&CaseInformationDialog::initDialog );
}

CaseInformationDialog::~CaseInformationDialog()
{
    delete ui;
}

void CaseInformationDialog::initDialog()
{
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
    }
}

void CaseInformationDialog::setDateAndTime()
{
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}

void CaseInformationDialog::openKeyboardPhysicianName()
{
    if(m_model.isSelectedPhysicianName()){
        const auto& list = m_model.physicianNames();
        const auto& value = m_model.selectedPhysicianName();
        int index = list.indexOf(value);
        QString paramName = ui->labelPhysicianName->text();
        QString paramValue = ui->lineEditPhysicianName->text();

        const std::vector<QString> param{paramName, paramValue};
        auto newName = WidgetContainer::instance()->openKeyboard(this, param, 400);
        ui->lineEditPhysicianName->setText(newName);
        m_model.setPhysicianName(index,newName);

        const bool isNext(!ui->lineEditPhysicianName->text().isEmpty());
        enableNext(isNext);
    } else {
        emit ui->pushButtonPhysicianNameDown->clicked();
    }
}

void CaseInformationDialog::openKeyboardPatientId()
{
    QString paramName = ui->labelPatientId->text();
    QString paramValue = ui->lineEditPatientId->text();

    const std::vector<QString> param{paramName, paramValue, "ENTER"};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditPatientId->setText(text);
    m_model.setPatientId(text);
}

void CaseInformationDialog::openKeyboardLocation()
{
    if(m_model.isSelectedLocation()){
        const auto& list = m_model.locations();
        const auto& value = m_model.selectedLocation();
        int index = list.indexOf(value);
        QString paramName = ui->labelLocation->text();
        QString paramValue = ui->lineEditLocation->text();

        const std::vector<QString> param{paramName, paramValue};
        auto newLocation = WidgetContainer::instance()->openKeyboard(this, param, 400);
        ui->lineEditLocation->setText(newLocation);
        m_model.setSelectedLocation(newLocation);
        m_model.setLocation(index, newLocation);
    } else {
        emit ui->pushButtonLocationDown->clicked();
    }
}

void CaseInformationDialog::on_pushButtonNext_clicked()
{
    m_displayTimer.stop();
    m_model.setDateAndTime(ui->lineEditDateAndTime->text());
    accept();
}

void CaseInformationDialog::enableNext(bool isNext)
{
    ui->pushButtonNext->setEnabled(isNext);
    if(isNext){
        ui->frame->setStyleSheet("background-color:#F5C400;");
    } else {
        ui->frame->setStyleSheet("background-color:#262626;");
    }
}

bool CaseInformationDialog::isFieldEmpty() const
{
    return !m_model.isSelectedPhysicianName();
}

void CaseInformationDialog::on_pushButtonPhysicianNameDown_clicked()
{
    auto* parent = this;
    m_selectDialog = new SelectDialog(parent);
    auto pw = parent->width();
    auto dw = m_selectDialog->width();
    int xVal = x() + pw/2 - dw/2 + 300;

    m_selectDialog->move(xVal, y() + 440);
    m_selectDialog->show();

    m_selectDialog->populate(m_model.physicianNames());

    if(m_selectDialog->exec() == QDialog::Accepted){
        m_model.setSelectedPhysicianName(m_selectDialog->selectedItem());
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
    } else {
        QString paramName = ui->labelPhysicianName->text();
        const ParameterType param{paramName, "", "ADD NEW"};
        auto newName = WidgetContainer::instance()->openKeyboard(this, param, 200);
        m_model.addPhysicianName(newName);
        m_model.setSelectedPhysicianName(newName);
        ui->lineEditPhysicianName->setText(m_model.selectedPhysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
    }
    const bool isNext(!ui->lineEditPhysicianName->text().isEmpty());
    enableNext(isNext);
}

void CaseInformationDialog::on_pushButtonLocationDown_clicked()
{
    auto* parent = this;
    m_selectDialog = new SelectDialog(parent);
    auto pw = parent->width();
    auto dw = m_selectDialog->width();
    int xVal = x() + pw/2 - dw/2 + 300;

    m_selectDialog->move(xVal, y() + 440);
    m_selectDialog->show();

    m_selectDialog->populate(m_model.locations());

    if(m_selectDialog->exec() == QDialog::Accepted){
        const auto& location =  m_selectDialog->selectedItem();
        ui->lineEditLocation->setText(location);
        m_model.setSelectedLocation(location);
//        ui->lineEditLocation->setStyleSheet("");
    } else {
        QString paramName = ui->labelLocation->text();
        const ParameterType param{paramName, "", "ADD NEW"};
        auto location = WidgetContainer::instance()->openKeyboard(this, param, 200);
        m_model.addLocation(location);
        ui->lineEditLocation->setText(location);
        m_model.setSelectedLocation(location);
    }
}
