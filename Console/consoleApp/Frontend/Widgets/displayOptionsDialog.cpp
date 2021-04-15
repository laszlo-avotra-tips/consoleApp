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

    const double scaleUp = 1.43; //lcv zomFactor
    QMatrix matrix = ui->graphicsView->matrix();
    ui->graphicsView->setTransform( QTransform::fromScale( scaleUp * matrix.m11(), scaleUp * matrix.m22() ) );

    move(0,0);

}

void DisplayOptionsDialog::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
        if(m_model){
            initSepiaGray();
        }
    }
}

void DisplayOptionsDialog::setModel(DisplayOptionsModel *model)
{
    m_model = model;

    if(m_model){
        const auto reticleBrightness = m_model->reticleBrightness();
        ui->horizontalSliderRingBrightness->setValue(reticleBrightness);
        initBrightnessAndContrast();
        initUpDown();
        initImagingDepth();
        if(m_scene){
            initSepiaGray();
        }
        m_model0 = *m_model;
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
    //restor the model
    *m_model = m_model0;
    SignalModel::instance()->setWhiteLevel(m_model0.imageContrast());
    SignalModel::instance()->setBlackLevel(m_model0.imageBrightness());

    userSettings &settings = userSettings::Instance();
    settings.setBrightness( m_model0.imageBrightness() );
    settings.setContrast(m_model0.imageContrast());

    //imaging depth
    setImagingDepth(m_model0.depthIndex());

    //image color
    updateGraySepiaSetting();

    //up/down
    updateDistalToProximalSetting(!m_model0.isPointedDown());

    //ring brightness
    settings.setReticleBrightness(m_model0.reticleBrightness());
    emit reticleBrightnessChanged();

    reject();
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
    if(m_scene){
        m_scene->paintOverlay();
    }
}


void DisplayOptionsDialog::on_horizontalSlider_valueChanged(int value)
{
    if(value > 0){
        m_depthIndex = value;
        setImagingDepth(value);
    } else {
        emit ui->horizontalSlider->valueChanged(1);
    }   
}

void DisplayOptionsDialog::on_horizontalSliderRingBrightness_valueChanged(int reticleBrightness)
{
    LOG1(reticleBrightness)
    userSettings::Instance().setReticleBrightness(reticleBrightness);
    m_model->setReticleBrightness(reticleBrightness);
    emit reticleBrightnessChanged();

    ui->labelReticleBrightness->setNum(reticleBrightness * 100 / 255);

    if(m_scene){
        m_scene->paintOverlay();
    }
}

void DisplayOptionsDialog::handleUp()
{
    if(m_model->isPointedDown()){
        m_model->setIsPointedDown(false);
    }
    updateUpDownButtonColor();
    updateDistalToProximalSetting(true);
}

void DisplayOptionsDialog::handleDown()
{
    if(!m_model->isPointedDown()){
        m_model->setIsPointedDown(true);
    }
    updateUpDownButtonColor();
    updateDistalToProximalSetting(false);
}

void DisplayOptionsDialog::updateUpDownButtonColor()
{
    const QString on("background-color: #F5C400;\nborder-radius: 30;");
    const QString off("background-color: #ffffff;\nborder-radius: 30;");

    if(m_model->isPointedDown()){
        ui->pushButtonUp->setStyleSheet(off);
        ui->pushButtonDown->setStyleSheet(on);
    } else {
         ui->pushButtonUp->setStyleSheet(on);
         ui->pushButtonDown->setStyleSheet(off);
    }
}

void DisplayOptionsDialog::updateDistalToProximalSetting(bool isUp)
{
    userSettings &settings = userSettings::Instance();
    if(isUp){
        settings.setCatheterView(userSettings::ProximalToDistal);
        LOG( INFO, "Catheter view: Up - proximal to distal" )
    } else {
        settings.setCatheterView(userSettings::DistalToProximal);
        LOG( INFO, "Catheter view: Down - distal to proximal" )
    }
}

void DisplayOptionsDialog::handleGray()
{
    if(!m_model->isImageColorGray()){
        m_model->setIsImageColorGray(true);
    }
    updateGraySepiaButtonColor();
    updateGraySepiaSetting();
}

void DisplayOptionsDialog::handleSepia()
{
    if(m_model->isImageColorGray()){
        m_model->setIsImageColorGray(false);
    }
    updateGraySepiaButtonColor();
    updateGraySepiaSetting();
}

void DisplayOptionsDialog::updateGraySepiaButtonColor()
{
    const QString on("background-color: #F5C400;\nborder-radius: 30;");
    const QString off("background-color: #ffffff;\nborder-radius: 30;");

    if(m_model->isImageColorGray()){
        ui->pushButtonGray->setStyleSheet(on);
        ui->pushButtonSepia->setStyleSheet(off);
    } else {
         ui->pushButtonGray->setStyleSheet(off);
         ui->pushButtonSepia->setStyleSheet(on);
    }
}

void DisplayOptionsDialog::updateGraySepiaSetting()
{
    if(m_model->isImageColorGray()){
        m_scene->loadColorModeGray();
    } else {
         m_scene->loadColorModeSepia();
    }
}

void DisplayOptionsDialog::initBrightnessAndContrast()
{
    connect(ui->horizontalSliderImageBrightness, &QSlider::valueChanged, this, &DisplayOptionsDialog::handleImageBrightness);
    connect(ui->horizontalSliderImageContrast, &QSlider::valueChanged, this, &DisplayOptionsDialog::handleImageContrast);

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

void DisplayOptionsDialog::initUpDown()
{
    connect(ui->pushButtonUp, &QPushButton::clicked, this, &DisplayOptionsDialog::handleUp);
    connect(ui->pushButtonDown, &QPushButton::clicked, this, &DisplayOptionsDialog::handleDown);

    userSettings &settings = userSettings::Instance();
    const bool isDown = settings.isDistalToProximalView();

    if(isDown)
    {
        emit ui->pushButtonDown->clicked();
    } else {
        emit ui->pushButtonUp->clicked();
    }
}

void DisplayOptionsDialog::initSepiaGray()
{
    connect(ui->pushButtonGray, &QPushButton::clicked, this, &DisplayOptionsDialog::handleGray);
    connect(ui->pushButtonSepia, &QPushButton::clicked, this, &DisplayOptionsDialog::handleSepia);

    const bool isGray = m_model->isImageColorGray();

    if(isGray){
        emit ui->pushButtonGray->clicked();
    } else {
        emit ui->pushButtonSepia->clicked();
    }
}

void DisplayOptionsDialog::initImagingDepth()
{
    if(m_model){
        m_depthIndex = m_model->depthIndex();
        emit ui->horizontalSlider->valueChanged(m_depthIndex);
    }
}

void DisplayOptionsDialog::handleImageContrast(int contrast)
{
    const int contrastPercent = 100 * (contrast + 255) / 510;
    ui->labelImageContrast->setNum(contrastPercent);
    SignalModel::instance()->setWhiteLevel(contrast);
    m_model->setImageContrast(contrast);
    userSettings &settings = userSettings::Instance();
    settings.setContrast( contrast );
    LOG1(contrast)

}

void DisplayOptionsDialog::handleImageBrightness(int brightness)
{
    const int brightnessPercent = 100 * (brightness + 255) / 510;
    ui->labelImageBrightness->setNum(brightnessPercent);
    SignalModel::instance()->setBlackLevel(brightness);
    m_model->setImageBrightness(brightness);
    userSettings &settings = userSettings::Instance();
    settings.setBrightness( brightness );
    LOG1(brightness)
}
