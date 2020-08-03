#include "displayOptionsDialog.h"
#include "ui_displayOptionsDialog.h"
#include "logger.h"
#include "signalmodel.h"
#include "Utility/userSettings.h"

DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DisplayOptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    userSettings &settings = userSettings::Instance();
    if(settings.isDistalToProximalView()){
        ui->radioButtonDown->setChecked(true);
    } else {
        ui->radioButtonUp->setChecked(true);
    }
}

DisplayOptionsDialog::~DisplayOptionsDialog()
{
    delete ui;
}

void DisplayOptionsDialog::on_pushButtonDone_clicked()
{
    accept();
}

void DisplayOptionsDialog::on_pushButtonBack_clicked()
{
    reject();
}

void DisplayOptionsDialog::on_radioButtonDown_clicked(bool checked)
{
    LOG1(checked)
    userSettings &settings = userSettings::Instance();

    if( checked )
    {
        settings.setCatheterView( userSettings::DistalToProximal );
        LOG( INFO, "Catheter view: Down - distal to proximal" )
    }
    else
    {
        settings.setCatheterView( userSettings::ProximalToDistal );
        LOG( INFO, "Catheter view: Up - proximal to distal" )
    }

//        emit updateCatheterView();
}

void DisplayOptionsDialog::on_radioButtonUp_clicked(bool checked)
{
    LOG1(checked)
//    SignalModel::instance()->setIsDistalToProximalView(true);
    userSettings &settings = userSettings::Instance();

    if( checked )
    {
        settings.setCatheterView( userSettings::ProximalToDistal );
        LOG( INFO, "Catheter view: Up - proximal to distal" )
    }
    else
    {
        settings.setCatheterView( userSettings::DistalToProximal );
        LOG( INFO, "Catheter view: Down - distal to proximal" )
    }
    //        emit updateCatheterView();
}

void DisplayOptionsDialog::on_radioButtonGrey_clicked(bool checked)
{
    LOG1(checked)
}

void DisplayOptionsDialog::on_radioButtonSepia_clicked(bool checked)
{
    LOG1(checked)
}
