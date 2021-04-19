#include "shutdownConfirmationDialog.h"
#include "ui_shutdownConfirmationDialog.h"
#include "displayManager.h"


ShutdownConfirmationDialog::ShutdownConfirmationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShutdownConfirmationDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);

    DisplayManager::instance()->showOnTheSecondMonitor("disk");
    DisplayManager::instance()->setWindowTitle("SHUTDOWN IN PROCESS");

}

ShutdownConfirmationDialog::~ShutdownConfirmationDialog()
{
    delete ui;
}
