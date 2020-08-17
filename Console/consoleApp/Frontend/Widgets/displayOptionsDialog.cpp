#include "displayOptionsDialog.h"
#include "ui_displayOptionsDialog.h"
#include "logger.h"
#include "signalmodel.h"
#include "Utility/userSettings.h"
#include "livescene.h"
#include "depthsetting.h"

#include <QGraphicsView>

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

    const double scaleUp = 1.5; //lcv zomFactor
    QMatrix matrix = ui->graphicsView->matrix();
    ui->graphicsView->setTransform( QTransform::fromScale( scaleUp * matrix.m11(), scaleUp * matrix.m22() ) );

    const auto reticleBrightness = userSettings::Instance().reticleBrightness();
    ui->horizontalSliderRingBrightness->setValue(reticleBrightness);
}

void DisplayOptionsDialog::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
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

void DisplayOptionsDialog::on_pushButtonDepthMimus_clicked()
{
    auto val = ui->horizontalSlider->value();
    if(val > 1 && val <= 5 ){
        int newVal = val - 1;
        if(m_depthIndex != newVal){
            m_depthIndex = newVal;
            ui->horizontalSlider->setValue(m_depthIndex);
//            ui->horizontalSlider->valueChanged(m_depthIndex);
            setImagingDepth(m_depthIndex);
        }
    }
}

void DisplayOptionsDialog::on_pushButtonDepthPlus_clicked()
{
    auto val = ui->horizontalSlider->value();
    if(val < 5 && val >= 1) {
        int newVal = val + 1;
        if(m_depthIndex != newVal){
            m_depthIndex = newVal;
            ui->horizontalSlider->setValue(m_depthIndex);
//            ui->horizontalSlider->valueChanged(m_depthIndex);
            setImagingDepth(m_depthIndex);
        }
    }
}

void DisplayOptionsDialog::setImagingDepth(int depthIndex)
{
    depthSetting &depthManager = depthSetting::Instance();
    depthManager.updateImagingDepth(m_imagingDepth[depthIndex]);

}


void DisplayOptionsDialog::on_horizontalSlider_valueChanged(int value)
{
    if(value > 0){
        m_depthIndex = value;
        setImagingDepth(value);
    } else {
        ui->horizontalSlider->setValue(1);
    }
}

void DisplayOptionsDialog::on_horizontalSliderRingBrightness_valueChanged(int value)
{
    userSettings::Instance().setReticleBrightness(value);
    emit reticleBrightnessChanged(value);
}

void DisplayOptionsDialog::on_horizontalSliderImageBrightness_valueChanged(int value)
{
    SignalModel::instance()->setWhiteLevel(value);
    userSettings &settings = userSettings::Instance();
    settings.setBrightness( value );
}


void DisplayOptionsDialog::on_horizontalSliderImageContrast_valueChanged(int value)
{
    SignalModel::instance()->setBlackLevel(value);
    userSettings &settings = userSettings::Instance();
    settings.setContrast( value );
}
