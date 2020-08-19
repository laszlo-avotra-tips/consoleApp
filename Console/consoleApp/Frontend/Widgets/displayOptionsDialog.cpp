#include "displayOptionsDialog.h"
#include "ui_displayOptionsDialog.h"
#include "logger.h"
#include "signalmodel.h"
#include "livescene.h"
#include "depthsetting.h"
#include "DisplayOptionsModel.h"
#include "Utility/userSettings.h"
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

    const double scaleUp = 1.43; //lcv zomFactor
    QMatrix matrix = ui->graphicsView->matrix();
    ui->graphicsView->setTransform( QTransform::fromScale( scaleUp * matrix.m11(), scaleUp * matrix.m22() ) );

}

void DisplayOptionsDialog::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
    }
}

void DisplayOptionsDialog::setModel(DisplayOptionsModel *model)
{
    m_model = model;

    if(m_model){
        const auto reticleBrightness = m_model->reticleBrightness();
        ui->horizontalSliderRingBrightness->setValue(reticleBrightness);
        initBrightnessAndContrast();
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
}

void DisplayOptionsDialog::on_radioButtonUp_clicked(bool checked)
{
    LOG1(checked)
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
    if(m_model){
        m_model->setIsImageColorGray(true);
    }
}

void DisplayOptionsDialog::on_radioButtonSepia_clicked(bool checked)
{
    LOG1(checked)
    m_scene->loadColorModeSepia();
    if(m_model){
        m_model->setIsImageColorGray(false);
    }
}

void DisplayOptionsDialog::on_pushButtonDepthMimus_clicked()
{
    auto val = ui->horizontalSlider->value();
    if(val > 1 && val <= 5 ){
        int newVal = val - 1;
        if(m_depthIndex != newVal){
            m_depthIndex = newVal;
            ui->horizontalSlider->setValue(m_depthIndex);
            setImagingDepth(m_depthIndex);
            m_model->setDepthIndex(m_depthIndex);
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
            setImagingDepth(m_depthIndex);
            m_model->setDepthIndex(m_depthIndex);
        }
    }
}

void DisplayOptionsDialog::setImagingDepth(int depthIndex)
{
    depthSetting &depthManager = depthSetting::Instance();
    depthManager.updateImagingDepth(m_imagingDepth[depthIndex]);
    m_depthIndex = depthIndex;
    m_model->setDepthIndex(m_depthIndex);
    ui->horizontalSlider->setValue(depthIndex);
}


void DisplayOptionsDialog::on_horizontalSlider_valueChanged(int value)
{
    if(value > 0){
        m_depthIndex = value;
    } else {
        m_depthIndex = 1;
    }
    setImagingDepth(value);
}

void DisplayOptionsDialog::on_horizontalSliderRingBrightness_valueChanged(int reticleBrightness)
{
    LOG1(reticleBrightness)
    userSettings::Instance().setReticleBrightness(reticleBrightness);
    m_model->setReticleBrightness(reticleBrightness);
    emit reticleBrightnessChanged(reticleBrightness);

    ui->labelReticleBrightness->setNum(reticleBrightness * 100 / 255);
}

void DisplayOptionsDialog::on_horizontalSliderImageBrightness_valueChanged(int brightness)
{
    SignalModel::instance()->setBlackLevel(brightness);
    m_model->setImageBrightness(brightness);
//    userSettings &settings = userSettings::Instance();
//    settings.setBrightness( brightness );
}


void DisplayOptionsDialog::on_horizontalSliderImageContrast_valueChanged(int contrast)
{
    SignalModel::instance()->setWhiteLevel(contrast);
    m_model->setImageContrast(contrast);
//    userSettings &settings = userSettings::Instance();
//    settings.setContrast( contrast );
}

void DisplayOptionsDialog::initBrightnessAndContrast()
{
    if(m_model){
        const auto& brightness = m_model->imageBrightness();
        const auto& contrast = m_model->imageContrast();

        LOG2(brightness, contrast)
        SignalModel::instance()->setWhiteLevel(contrast);
        SignalModel::instance()->setBlackLevel(brightness);

        ui->horizontalSliderImageBrightness->setValue(brightness);
        ui->horizontalSliderImageContrast->setValue(contrast);
    }
}
