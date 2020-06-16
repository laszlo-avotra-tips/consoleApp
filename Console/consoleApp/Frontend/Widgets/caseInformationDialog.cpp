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
//    setDateAndTime();

//    connect(&m_displayTimer, &QTimer::timeout, this, &CaseInformationDialog::setDateAndTime);
//    m_displayTimer.start(500);

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
    if(!isFieldEmpty()){
        QString paramName = ui->labelPhysicianName->text();
        QString paramValue = ui->lineEditPhysicianName->text();

        const std::vector<QString> param{paramName, paramValue};
        auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
        ui->lineEditPhysicianName->setText(text);

        const bool isNext(!ui->lineEditPhysicianName->text().isEmpty());
        enableNext(isNext);
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
    QString paramName = ui->labelLocation->text();
    QString paramValue = ui->lineEditLocation->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditLocation->setText(text);
    m_model.setSelectedLocation(text);
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
//        ui->pushButtonNext->setStyleSheet("QPushButton{background-color:#F5C400;}");
        auto& button = ui->pushButtonNext;
        QPalette pal = button->palette();
        pal.setColor(QPalette::Button, QColor(245,196,0));
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
    }
}

bool CaseInformationDialog::isFieldEmpty() const
{
//    return ui->lineEditPhysicianName->text() == QString("Required field");
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

//    m_selectDialog->update(PhysicianNameModel::instance()->physicianNames());
    m_selectDialog->update(m_model.physicianNames());

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

//    m_selectDialog->update(LocationModel::instance()->locations());
    m_selectDialog->update(m_model.locations());

    if(m_selectDialog->exec() == QDialog::Accepted){
        ui->lineEditLocation->setText(m_selectDialog->selectedItem());
        ui->lineEditLocation->setStyleSheet("");
    } else {
        QString paramName = ui->labelLocation->text();
        const ParameterType param{paramName, "", "ADD NEW"};
        auto text = WidgetContainer::instance()->openKeyboard(this, param, 200);
        ui->lineEditLocation->setText(text);
    }
}
\
