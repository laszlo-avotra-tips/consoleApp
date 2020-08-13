#include "displayOptionsDialog.h"
#include "ui_displayOptionsDialog.h"
#include "logger.h"
#include "signalmodel.h"
#include "Utility/userSettings.h"
#include <QGraphicsView>
#include "livescene.h"

DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DisplayOptionsDialog)
{
    ui->setupUi(this);
    m_graphicsView = ui->graphicsView;

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    userSettings &settings = userSettings::Instance();
    if(settings.isDistalToProximalView()){
        ui->radioButtonDown->setChecked(true);
    } else {
        ui->radioButtonUp->setChecked(true);
    }
}

void DisplayOptionsDialog::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
//        emit ui->radioButtonGrey->clicked(true); //initial state
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
    m_scene->loadColorModeGray();
}

void DisplayOptionsDialog::on_radioButtonSepia_clicked(bool checked)
{
    LOG1(checked)
    m_scene->loadColorModeSepia();
}
