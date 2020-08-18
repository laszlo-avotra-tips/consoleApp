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

    initBrightnessAndContrast();
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

void DisplayOptionsDialog::on_horizontalSliderRingBrightness_valueChanged(int reticleBrightness)
{
    userSettings::Instance().setReticleBrightness(reticleBrightness);
    emit reticleBrightnessChanged(reticleBrightness);

    ui->groupBoxRingBrightness->setTitle("RING BRIGHTNESS: " + QString::number(reticleBrightness * 100 / 255 ) + QString("%"));
}

void DisplayOptionsDialog::on_horizontalSliderImageBrightness_valueChanged(int brightness)
{
    SignalModel::instance()->setBlackLevel(brightness);
    userSettings &settings = userSettings::Instance();
    settings.setBrightness( brightness );
    ui->groupBoxImageBrightness->setTitle("IMAGE BRIGHTNESS: " +  QString::number((brightness + 255) * 100 / 510 )  + QString("%"));
}


void DisplayOptionsDialog::on_horizontalSliderImageContrast_valueChanged(int contrast)
{
    SignalModel::instance()->setWhiteLevel(contrast);
    userSettings &settings = userSettings::Instance();
    settings.setContrast( contrast );

    ui->groupBoxImageContrast->setTitle("IMAGE CONTRAST: " +  QString::number((contrast + 255) * 100 / 510 )  + QString("%"));
}

void DisplayOptionsDialog::initBrightnessAndContrast()
{
    userSettings &settings = userSettings::Instance();
    const auto& brightness = settings.brightness();
    const auto& contrast = settings.contrast();

    ui->horizontalSliderImageBrightness->setValue(brightness);
    ui->horizontalSliderImageContrast->setValue(contrast);
}
