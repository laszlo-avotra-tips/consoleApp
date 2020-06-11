#include "caseInformationDialog.h"
#include "ui_caseInformationDialog.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "consoleLineEdit.h"
#include "selectDialog.h"
#include "physicianNameModel.h"

#include <QDateTime>

CaseInformationDialog::CaseInformationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseInformationDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    setDateAndTime();
    connect(&m_displayTimer, &QTimer::timeout, this, &CaseInformationDialog::setDateAndTime);
    m_displayTimer.start(500);
    connect(ui->lineEditPhysicianName, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardPhysicianName);
    connect(ui->lineEditPatientId, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardPatientId);
    connect(ui->lineEditLocation, &ConsoleLineEdit::mousePressed, this, &CaseInformationDialog::openKeyboardLocation);

    if(ui->lineEditPhysicianName->text().isEmpty()){
        ui->pushButtonNext->setEnabled(false);
    }
}

CaseInformationDialog::~CaseInformationDialog()
{
    delete ui;
}

void CaseInformationDialog::setDateAndTime()
{
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}

void CaseInformationDialog::openKeyboardPhysicianName()
{
    QString paramName = ui->labelPhysicianName->text();
    QString paramValue = ui->lineEditPhysicianName->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditPhysicianName->setText(text);

    const bool isNext(!ui->lineEditPhysicianName->text().isEmpty());
    enableNext(isNext);
}

void CaseInformationDialog::openKeyboardPatientId()
{
    QString paramName = ui->labelPatientId->text();
    QString paramValue = ui->lineEditPatientId->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditPatientId->setText(text);
}

void CaseInformationDialog::openKeyboardLocation()
{
    QString paramName = ui->labelLocation->text();
    QString paramValue = ui->lineEditLocation->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditLocation->setText(text);
}

void CaseInformationDialog::on_pushButtonNext_clicked()
{
    m_displayTimer.stop();
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

void CaseInformationDialog::on_pushButtonPhysicianNameDown_clicked()
{
    auto* parent = this;
    m_selectDialog = new SelectDialog(parent);
    auto pw = parent->width();
    auto dw = m_selectDialog->width();
    int xVal = x() + pw/2 - dw/2 + 200;

    m_selectDialog->move(xVal, y() + 200);
    m_selectDialog->show();

    m_selectDialog->update(PhysicianNameModel::instance()->physicianNames());

    if(m_selectDialog->exec() == QDialog::Accepted){
        ui->lineEditPhysicianName->setText(PhysicianNameModel::instance()->selectedPysicianName());
        ui->lineEditPhysicianName->setStyleSheet("");
    } else {
        QString paramName = ui->labelPhysicianName->text();
        const ParameterType param{paramName, "", "ADD NEW"};
        auto text = WidgetContainer::instance()->openKeyboard(this, param, 200);
        ui->lineEditPhysicianName->setText(text);
    }

}

void CaseInformationDialog::on_pushButtonLocationDown_clicked()
{

}
