#include "dialogcaseinformation.h"
#include "ui_dialogcaseinformation.h"

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
}

DialogCaseInformation::~DialogCaseInformation()
{
    delete ui;
}

void DialogCaseInformation::setDateAndTime()
{
    ui->lineEditDateAndTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}
