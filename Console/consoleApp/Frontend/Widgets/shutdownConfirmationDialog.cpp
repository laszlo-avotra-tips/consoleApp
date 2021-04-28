#include "shutdownConfirmationDialog.h"
#include "ui_shutdownConfirmationDialog.h"
#include "displayManager.h"
#include "logger.h"


ShutdownConfirmationDialog::ShutdownConfirmationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShutdownConfirmationDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    DisplayManager::instance()->showOnTheSecondMonitor("disk");
    DisplayManager::instance()->setWindowTitle("SHUTDOWN IN PROCESS");
    LOGUA;

}

ShutdownConfirmationDialog::~ShutdownConfirmationDialog()
{
    LOGUA;
    delete ui;
}
