#include "dialogcaseinformation.h"
#include "ui_dialogcaseinformation.h"
#include "Frontend/Utility/widgetcontainer.h"
#include "octlineedit.h"

#include <QDateTime>

DialogCaseInformation::DialogCaseInformation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCaseInformation)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    setDateAndTime();
    connect(&m_displayTimer, &QTimer::timeout, this, &DialogCaseInformation::setDateAndTime);
    m_displayTimer.start(500);
    connect(ui->lineEditPhysicianName, &OctLineEdit::mousePressed, this, &DialogCaseInformation::openKeyboardPhysicianName);
    connect(ui->lineEditPatientId, &OctLineEdit::mousePressed, this, &DialogCaseInformation::openKeyboardPatientId);
    connect(ui->lineEditLocation, &OctLineEdit::mousePressed, this, &DialogCaseInformation::openKeyboardLocation);
}

DialogCaseInformation::~DialogCaseInformation()
{
    delete ui;
}

void DialogCaseInformation::setDateAndTime()
{
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}

void DialogCaseInformation::openKeyboardPhysicianName()
{
    QString paramName = ui->labelPhysicianName->text();
    QString paramValue = ui->lineEditPhysicianName->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditPhysicianName->setText(text);
}

void DialogCaseInformation::openKeyboardPatientId()
{
    QString paramName = ui->labelPatientId->text();
    QString paramValue = ui->lineEditPatientId->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditPatientId->setText(text);
}

void DialogCaseInformation::openKeyboardLocation()
{
    QString paramName = ui->labelLocation->text();
    QString paramValue = ui->lineEditLocation->text();

    const std::vector<QString> param{paramName, paramValue};
    auto text = WidgetContainer::instance()->openKeyboard(this, param, 400);
    ui->lineEditLocation->setText(text);
}
