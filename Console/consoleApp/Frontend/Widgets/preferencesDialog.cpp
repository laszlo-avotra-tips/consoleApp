#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}
