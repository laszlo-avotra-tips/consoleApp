#include "mainScreen.h"
#include "ui_mainScreen.h"
#include "Utility/widgetcontainer.h"
#include "Utility/screenFactory.h"
#include "deviceSettings.h"
#include "logger.h"
#include "opaqueScreen.h"
#include "Widgets/caseInformationDialog.h"
#include "Widgets/caseInformationModel.h"
#include "Widgets/reviewAndSettingsDialog.h"
#include "Utility/captureListModel.h"
#include "Utility/octFrameRecorder.h"
#include "displayOptionsDialog.h"
#include "DisplayOptionsModel.h"
#include "livescene.h"
#include "scanconversion.h"
#include "signalmodel.h"
#include "daqfactory.h"
#include "idaq.h"
#include "Utility/userSettings.h"
#include "Utility/clipListModel.h"
#include "displayManager.h"
#include "defaults.h"
#include <Backend/interfacesupport.h>
#include "endCaseDialog.h"

#include <QTimer>
#include <QDebug>
#include <QLayoutItem>
#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QGraphicsView>
#include <QBitmap>
#include <memory>

MainScreen::MainScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainScreen)
{
    ui->setupUi(this);

    m_graphicsView = ui->graphicsView;

    m_navigationButtons.push_back(ui->pushButtonEndCase);
    m_navigationButtons.push_back(ui->pushButtonSettings);
    m_navigationButtons.push_back(ui->pushButtonMeasure);
    m_navigationButtons.push_back(ui->pushButtonRecord);
    m_navigationButtons.push_back(ui->pushButtonCapture);
    m_navigationButtons.push_back(ui->pushButtonFlip);

    ui->pushButtonDownArrow->hide();
    ui->pushButtonCondensUp->show();

    m_updateTimeTimer.start(m_updateTimeTimeoutMs);
    connect(&m_updateTimeTimer, &QTimer::timeout, this, &MainScreen::updateTime);

    m_opacScreen = new OpaqueScreen(this);
    m_opacScreen->show();
    m_graphicsView->hide();
    ui->frameSpeed->hide();

    connect(ui->pushButtonLow, &QPushButton::clicked, this, &MainScreen::udpateToSpeed1);
    connect(ui->pushButtonMedium, &QPushButton::clicked, this, &MainScreen::udpateToSpeed2);
    connect(ui->pushButtonHigh, &QPushButton::clicked, this, &MainScreen::udpateToSpeed3);
    connect(this, &MainScreen::sledRunningStateChanged, this, &MainScreen::handleSledRunningState);
    connect(&m_daqTimer, &QTimer::timeout, this, &MainScreen::updateImage );

    QMatrix matrix = ui->graphicsView->matrix();
    ui->graphicsView->setTransform( QTransform::fromScale( IMAGE_SCALE_FACTOR * matrix.m11(), IMAGE_SCALE_FACTOR * matrix.m22() ) );

    m_scene = new liveScene( this );
    m_graphicsView->setScene(m_scene);
    DisplayManager::instance()->setScene(m_scene);
    m_scene->handleReticleBrightnessChanged();

    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_clipBuffer = new uint8_t[1024 * 1024];
    hookupEndCaseDiagnostics();
//    ui->pushButton->setEnabled(false);
}

void MainScreen::hookupEndCaseDiagnostics() {
    diagnostics = new EndCaseDiagnostics();
    auto messageBox = styledMessageBox::instance(); //new PowerUpMessageBox();

    QObject::connect(diagnostics, &OctSystemDiagnostics::showMessageBox,
                     messageBox, &styledMessageBox::onShowMessageBox);
    QObject::connect(diagnostics, &OctSystemDiagnostics::hideMessageBox,
                     messageBox, &styledMessageBox::onHideMessageBox);

    QObject::connect(messageBox, &styledMessageBox::userAcknowledged,
                     diagnostics, &OctSystemDiagnostics::onUserAcknowledged);
    LOG(INFO, "End case diagnostics framework initialized");
}

void MainScreen::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
        daqfactory::instance()->getdaq();
        DisplayManager::instance()->setScene(m_scene);
    }
}

MainScreen::~MainScreen()
{
    delete[] m_clipBuffer;
    delete ui;
}

bool MainScreen::isVisible() const
{
    const auto& region = m_graphicsView->visibleRegion();
    return !region.isEmpty();
}

void MainScreen::on_pushButtonFlip_clicked()
{
    flipColumns();
}


void MainScreen::on_pushButtonMenu_clicked()
{
    toggleNavigationButtons(m_navigationButtons);
}

void MainScreen::flipColumns()
{
    QLayout* tl = layout();
    std::vector<QLayoutItem*> current{tl->itemAt(0),tl->itemAt(1),tl->itemAt(2)};

    tl->removeItem(current[2]);
    tl->removeItem(current[1]);
    tl->removeItem(current[0]);
    tl->addItem(current[2]);
    tl->addItem(current[1]);
    tl->addItem(current[0]);
    tl->update();
}

void MainScreen::toggleNavigationButtons(const std::vector<QWidget *> &buttons)
{
    if(buttons[0]->isVisible()){
        for(auto* button : buttons){
            button->hide();
        }
        ui->pushButtonDownArrow->show();
        ui->pushButtonCondensUp->hide();
    }else {
        for(auto* button : buttons){
            button->show();
        }
        ui->pushButtonDownArrow->hide();
        ui->pushButtonCondensUp->show();
    }
}

void MainScreen::setCurrentTime()
{
    m_currentTime = QTime::currentTime();
    QString timeString = m_currentTime.toString("hh:mm:ss");
    ui->labelCurrentTime->setText(timeString);
    DisplayManager::instance()->setCurrentTime(timeString);
}

void MainScreen::setSpeedAndEnableDisableBidirectional(int speed)
{
    if(speed >= 600){
        LOG1(speed);

        auto idaq = daqfactory::instance()->getdaq();
        if(idaq){
            idaq->setSubsamplingAndForcedTrigger(speed);
        }

        auto interfaceSupport = InterfaceSupport::getInstance();
        interfaceSupport->setSledSpeed(speed);
        deviceSettings &device = deviceSettings::Instance();
        auto currentDev = device.current();
        const bool isBiDirectionalEnabled{currentDev->isBiDirectional()};
        interfaceSupport->enableDisableBidirectional(isBiDirectionalEnabled);
    }
}

void MainScreen::highlightSpeedButton(QPushButton *wid)
{
    ui->pushButtonLow->setStyleSheet("");
    ui->pushButtonMedium->setStyleSheet("");
    ui->pushButtonHigh->setStyleSheet("");

    wid->setStyleSheet("background-color: #F5C400; color: black;");
}

int MainScreen::getSledRuntime()
{
    updateSledRunningState();

    if(m_runTime.isValid()){
        int delta{0};
        if((m_sledRunningStateVal == 1) || (m_sledRunningStateVal == 3)){
            delta = m_runTime.restart();
            m_sledRuntime += delta;
        } else {
            m_runTime.start();
        }
//        LOG3(m_sledRunningStateVal,delta, m_sledRuntime);
    }
    return m_sledRuntime;
}

int MainScreen::getSceneWidth()
{
    int retVal = m_sceneWidth;
    QString fn("/Avinger_System/screen.dat");
    QFile sf(fn);
    if(sf.open(QIODevice::ReadOnly)){
        QTextStream ts(&sf);
        int ratio{1};
        int isFullScreen;
        ts >> ratio >> isFullScreen;
        if(ratio){
            int width = m_sceneWidth / ratio;
            qDebug() << fn << " open ok. width = " << width << ", isFullScreen " << isFullScreen;
            WidgetContainer::instance()->setIsFullScreen(isFullScreen);
            WidgetContainer::instance()->setMiddleFrameSize(width);
            WidgetContainer::instance()->setRatio(ratio);
            retVal = width;
        }
        sf.close();
    }

    return retVal;
}

QSize MainScreen::getSceneSize()
{
    return m_sceneSize;
}

void MainScreen::on_pushButtonEndCase_clicked()
{
    auto dialog = new endCaseDialog(this);

    if(dialog->exec() == QDialog::Accepted){
        handleEndCase();
    } else {
        dialog->hide();
        DisplayManager::instance()->showOnTheSecondMonitor("liveData");
    }
    delete dialog;
}

void MainScreen::handleEndCase()
{

    DisplayManager::instance()->showOnTheSecondMonitor("logo");
    if(m_recordingIsOn){
        LOG1(m_recordingIsOn)
        ui->pushButtonRecord->click();
    }

    auto idaq = daqfactory::instance()->getdaq();
    bool isDisonnected = disconnect( idaq, &IDAQ::updateSector, this, &MainScreen::updateSector);
    LOG1(isDisonnected);

    QTimer::singleShot(1000, [this](){
        m_opacScreen->show();
        m_graphicsView->hide();

        daqfactory::instance()->getdaq()->shutdownDaq();

        WidgetContainer::instance()->gotoScreen("startScreen");

        WidgetContainer::instance()->unRegisterWidget("l2500Frontend");

        m_sledRuntime = 0;
        m_runTime.invalidate();

        m_updateTimeTimer.stop();
        ui->labelRunTime->setText(QString("Runtime: 0:00:00"));
        ui->frameSpeed->hide();
        DisplayManager::instance()->setSpeedVisible(false);

        captureListModel::Instance().reset();
        clipListModel::Instance().reset();
        LOG1(m_recordingIsOn)
    });

    if (diagnostics) {
        if (!diagnostics->performDiagnostics(true)) {
            LOG(ERROR, "End case diagnostics failed!");
            return;
        } else {
            LOG(INFO, "End case diagnostics succeeded!");
        }
    }
}

void MainScreen::updateMainScreenLabels(const OCTFile::OctData_t &frameData)
{
    QString activePassiveValue{"ACTIVE"};
    if(m_sledRunningState != m_sledRunningStateVal){
        m_sledRunningState = m_sledRunningStateVal;
        if(m_sledRunningState == 3)
        {
           activePassiveValue = "PASSIVE";
        }
        else if(m_sledRunningState == 1)
        {
           activePassiveValue = "ACTIVE";
        }

        if(m_scene){
            m_scene->paintOverlay();
        }
    }
    const QDateTime currentTime = QDateTime::currentDateTime();
    const QString timeLabel{currentTime.toString("hh:mm:ss")};
    const auto& dev = deviceSettings::Instance().current();

    if(!dev->isBiDirectional()){
        activePassiveValue = QString("");
    }

    auto devName = dev->getSplitDeviceName();
    QStringList names = devName.split("\n");
    const QString catheterName{names[0]};
    const QString cathalogName{names[1]};

    emit updateRecorder(frameData.dispData,
                        catheterName.toLatin1(),cathalogName.toLatin1(),
                        activePassiveValue.toLatin1(),
                        timeLabel.toLatin1(),
                        1024,1024);
}

void MainScreen::on_pushButtonDownArrow_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainScreen::on_pushButtonCondensUp_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainScreen::handleYellowBorder()
{
    QString borderStyleSheet;
    if(m_recordingIsOn){
        borderStyleSheet = QString("border:1px solid rgb(245,196,0);");
    } else {
        borderStyleSheet = QString("border:1px solid rgb(0,0,0);");
    }
    ui->graphicsView->setStyleSheet(borderStyleSheet);
    DisplayManager::instance()->setBorderForRecording(borderStyleSheet);
}

void MainScreen::setDeviceLabel()
{
    ui->labelDevice->show();
    ui->labelRunTime->show();

    deviceSettings &dev = deviceSettings::Instance();
    const QString name{dev.getCurrentSplitDeviceName()};
    ui->labelDevice->setText(name);
    m_opacScreen->hide();
    m_graphicsView->show();
    m_runTime.start();
    m_updateTimeTimer.start(m_updateTimeTimeoutMs);
    updateTime();
}

void MainScreen::showSpeed(bool isShown)
{
    ui->frameSpeed->setVisible(isShown);
    DisplayManager::instance()->setSpeedVisible(isShown);
}

void MainScreen::on_pushButtonSettings_clicked()
{
    m_opacScreen->show();
    auto result = WidgetContainer::instance()->openDialog(this, "reviewAndSettingsDialog");//page. 58
    m_opacScreen->hide();
    LOG1(result.first)
    if(result.first){
        result.first->hide();
    }
    if( result.second == QDialog::Accepted){
        auto * dialog = result.first;
        const auto& reviewAndSettingsSelection = dialog->windowTitle();

        if(reviewAndSettingsSelection.trimmed() == "DISPLAY OPTIONS"){
            openDisplayOptionsDialog();
        }
        if(reviewAndSettingsSelection.trimmed() == "CASE INFORMATION"){
            m_opacScreen->show();
            openCaseInformationDialogFromReviewAndSettings();
            m_opacScreen->hide();
        }
        if(reviewAndSettingsSelection.trimmed() == "DEVICE SELECT"){
            m_opacScreen->show();
            openDeviceSelectDialogFromReviewAndSettings();
            m_opacScreen->hide();
        }
        if(reviewAndSettingsSelection.trimmed() == "CASE REVIEW"){
            LOG1("CASE REVIEW")
            WidgetContainer::instance()->gotoScreen("caseReviewScreen");
            if(result.first){
                delete result.first;
            }

        }
    }
    else {
       LOG1 ("Cancelled");
    }
}

void MainScreen::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    qDebug() << __FUNCTION__;
    if(WidgetContainer::instance()->getIsNewCase()){
        CaseInformationModel::instance()->init();
        QTimer::singleShot(100,this, &MainScreen::openCaseInformationDialog);
        //clear sector
    }
    m_scene->clearOverlay();
    QImage* image = m_scene->sectorImage();

    LOG1(image)
    if(image){
        memset(image->bits(), 0, 1024*1024);
        QGraphicsPixmapItem* pixmap = m_scene->sectorHandle();

        if(pixmap){
            QPixmap tmpPixmap = QPixmap::fromImage( *image, Qt::MonoOnly);
            pixmap->setPixmap(tmpPixmap);
        }
        bool isSimulation = userSettings::Instance().getIsSimulation();
        LOG1(isSimulation)
        if(!isSimulation){
            ui->labelSim->hide();
        }
    }
}

void MainScreen::hideEvent(QHideEvent *he)
{
    QWidget::hideEvent( he );
    qDebug() << __FUNCTION__;
    WidgetContainer::instance()->setIsNewCase(false);
}

void MainScreen::openCaseInformationDialog()
{
    const std::vector<QString> cidParam{"NEXT"};
    auto result = WidgetContainer::instance()->openDialog(this,"caseInformationDialog");

    if( result.second == QDialog::Accepted){
        qDebug() << "Accepted";
        openDeviceSelectDialog();
    }
    else {
        qDebug() << "Cancelled";
        WidgetContainer::instance()->gotoScreen("startScreen");
    }
}

void MainScreen::openCaseInformationDialogFromReviewAndSettings()
{
    CaseInformationModel model = *CaseInformationModel::instance();
    const std::vector<QString> cidParam{"DONE"};
    auto result = WidgetContainer::instance()->openDialog(this, "caseInformationDialog", &cidParam);

    if(result.first){
        result.first->hide();
    }
    if( result.second != QDialog::Accepted){
        *CaseInformationModel::instance() = model;
        on_pushButtonSettings_clicked();
    }
}

void MainScreen::updateDeviceSettings()
{
    deviceSettings &dev = deviceSettings::Instance();
    auto selectedDevice = dev.current();

    if(!selectedDevice){
        return;
    }

    InterfaceSupport::getInstance()->updateSledConfig(*selectedDevice);

    const bool isBidir = selectedDevice->isBiDirectional();
    const int numberOfSpeeds = selectedDevice->getNumberOfSpeeds();

    auto interfaceSupport = InterfaceSupport::getInstance();
    int currentSledRunningStateVal{interfaceSupport->getRunningState()};

    emit sledRunningStateChanged(currentSledRunningStateVal);

    if(isBidir){
        m_scene->setActive();
    } else {
        m_scene->setIdle();
    }

    int speedIndex = selectedDevice->getDefaultSpeedIndex();
    LOG3(isBidir, numberOfSpeeds, speedIndex)

    if(numberOfSpeeds == 3){
        switch(speedIndex){
        case 1:
            emit ui->pushButtonLow->clicked();
            break;
        case 2:
            emit ui->pushButtonMedium->clicked();
            break;

        case 3:
            emit ui->pushButtonHigh->clicked();
            break;
        }
    } else {
         setSpeedAndEnableDisableBidirectional(selectedDevice->getRevolutionsPerMin1());
    }
}

void MainScreen::showYellowBorderForRecordingOn(bool recordingIsOn)
{
    m_recordingIsOn = recordingIsOn;
    LOG1(m_recordingIsOn)
    QString borderStyleSheet;
    if(m_recordingIsOn){
        borderStyleSheet = QString("border:1px solid rgb(245,196,0);");
    } else {
        borderStyleSheet = QString("border:1px solid rgb(0,0,0);");
    }
    ui->graphicsView->setStyleSheet(borderStyleSheet);
    DisplayManager::instance()->setBorderForRecording(borderStyleSheet);
}

void MainScreen::openDeviceSelectDialog()
{
    auto result = WidgetContainer::instance()->openDialog(this,"deviceSelectDialog");

    if( result.second == QDialog::Accepted){
        LOG1( "Accepted")

        updateDeviceSettings();

        auto model = std::make_unique<DisplayOptionsModel>();
        auto dialog = std::make_unique<DisplayOptionsDialog>();
        dialog->setModel(model.get());
        dialog->setScene(m_scene);
        model->persistModel();
        m_scene->handleDeviceChange();
        deviceSettings &dev = deviceSettings::Instance();
        auto selectedDevice = dev.current();
        DisplayManager::instance()->setDevice(selectedDevice->getSplitDeviceName());
        m_daqTimer.start(1);
        DisplayManager::instance()->showOnTheSecondMonitor("liveData");

    } else {
        LOG1( "Cancelled")
        openCaseInformationDialog();
    }
}

void MainScreen::openDeviceSelectDialogFromReviewAndSettings()
{
    const std::vector<QString> dsdParam{"NO ANIMATION"};
    auto result = WidgetContainer::instance()->openDialog(this,"deviceSelectDialog",&dsdParam);

    if( result.second != QDialog::Accepted){
        on_pushButtonSettings_clicked();
    } else {
        updateDeviceSettings();
        deviceSettings &dev = deviceSettings::Instance();
        auto selectedDevice = dev.current();
        DisplayManager::instance()->setDevice(selectedDevice->getSplitDeviceName());
        DisplayManager::instance()->showOnTheSecondMonitor("liveData");
    }
    m_scene->paintOverlay();
}

void MainScreen::openDisplayOptionsDialog()
{
    int result{-1};

    auto model = DisplayOptionsModel();
    auto dialog = new DisplayOptionsDialog(this);
    dialog->setModel(&model);

    if(dialog){
        dialog->setScene(m_scene);

        connect( dialog, &DisplayOptionsDialog::reticleBrightnessChanged,
                 m_scene,     &liveScene::handleReticleBrightnessChanged );

        dialog->show();
        result = dialog->exec();

        if( result == QDialog::Accepted){
            model.persistModel();
            m_scene->handleDeviceChange();
        } else {
            on_pushButtonSettings_clicked();
        }
        m_scene->paintOverlay();
    }
}

void MainScreen::updateTime()
{
    int sledRunTime{getSledRuntime()};

    if(sledRunTime){
        int durationInSec = sledRunTime / 1000;
        //to test rollover
//        durationInSec += 3600 * 9 + 60 * 59;
        int sec = durationInSec % 60;
        int min = (durationInSec % 3600) / 60;
        int hour = durationInSec / 3600;
//        LOG4(durationInSec, hour , min, sec)
        QTime dt(hour,min,sec,0);

        QString elapsed = dt.toString("hh:mm:ss");
        const auto runtimeDisplay = elapsed.remove(0,1);
//        LOG3(runtimeDisplay,runtimeDisplay.isEmpty(),m_runTime.isValid())

        if(runtimeDisplay.isEmpty() || !m_runTime.isValid()){
             ui->labelRunTime->setText(QString("Runtime: 0:00:00"));
             DisplayManager::instance()->setRuntimeLabel(QString("Runtime: 0:00:00"));
        }else {
            ui->labelRunTime->setText(QString("Runtime: ") + runtimeDisplay);
            DisplayManager::instance()->setRuntimeLabel(QString("Runtime: ") + runtimeDisplay);
        }
    }

    setCurrentTime();
}

void MainScreen::udpateToSpeed1()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin1();

    setSpeedAndEnableDisableBidirectional(speed);
    highlightSpeedButton(ui->pushButtonLow);
    DisplayManager::instance()->setSpeed("LOW");
}

void MainScreen::udpateToSpeed2()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin2();

    setSpeedAndEnableDisableBidirectional(speed);
    highlightSpeedButton(ui->pushButtonMedium);
    DisplayManager::instance()->setSpeed("MEDIUM");
}

void MainScreen::udpateToSpeed3()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin3();

    setSpeedAndEnableDisableBidirectional(speed);
    highlightSpeedButton(ui->pushButtonHigh);
    DisplayManager::instance()->setSpeed("HIGH");

}

void MainScreen::on_pushButtonCapture_released()
{
    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QString blackBorder("border:5px solid rgb(0,0,0);");
    ui->graphicsView->setStyleSheet(blackBorder);
    DisplayManager::instance()->setBorderForRecording(blackBorder);

    onCaptureImage();

    QString yellowBorder("border:2px solid rgb(245,196,0);");
    ui->graphicsView->setStyleSheet(yellowBorder);
    DisplayManager::instance()->setBorderForRecording(yellowBorder);

    QTimer::singleShot(500,this,&MainScreen::handleYellowBorder);
}

void MainScreen::on_pushButtonMeasure_clicked(bool checked)
{
    setMeasurementMode(checked);
}

void MainScreen::updateSledRunningState()
{
    auto interfaceSupport = InterfaceSupport::getInstance();
    int currentSledRunningStateVal{interfaceSupport->getRunningState()};

     if(m_sledRunningStateVal != currentSledRunningStateVal)
     {
         m_sledRunningStateVal = currentSledRunningStateVal;
         emit sledRunningStateChanged(m_sledRunningStateVal);
     }
}

void MainScreen::handleSledRunningState(int runningStateVal)
{
    m_sledIsInRunningState = (runningStateVal == 1) || (runningStateVal == 3);

    auto interfaceSupport = InterfaceSupport::getInstance();
    auto idaq = daqfactory::instance()->getdaq();
    if(m_sledIsInRunningState){
        auto laserOnSuccess = idaq->turnLaserOn();
        LOG2(m_sledIsInRunningState,laserOnSuccess)
        interfaceSupport->setVOAMode(true);
    } else {
        auto laserOffSuccess = idaq->turnLaserOff();
        LOG2(m_sledIsInRunningState,laserOffSuccess)
        interfaceSupport->setVOAMode(false);
    }

    auto&ds = deviceSettings::Instance();
    auto device = ds.current();

    QString labelLiveColor;
    if(device && m_scene){
        const bool isBd = device->isBiDirectional();

        if(runningStateVal == 1){
            labelLiveColor = QString("color: green;");
            if(isBd){
                m_scene->setActive();
                DisplayManager::instance()->setSpeedVisible(true);
            }
        } else if (runningStateVal == 3){
            labelLiveColor = QString("color: green;");
            if(isBd){
                m_scene->setPassive();
                DisplayManager::instance()->setSpeedVisible(true);
            }
        }else{
            labelLiveColor = QString("color: grey;");
            m_scene->setIdle();
        }
        ui->labelLive->setStyleSheet(labelLiveColor);
        DisplayManager::instance()->setLabelLiveColor(labelLiveColor);
        if(m_sledIsInRunningState && ui->pushButtonMeasure->isChecked()){
            on_pushButtonMeasure_clicked(false);
        }

        ui->pushButtonMeasure->setEnabled(!m_sledIsInRunningState);
    }
}

void MainScreen::on_pushButtonRecord_clicked(bool checked)
{

    if(checked != m_recordingIsOn){
        m_recordingIsOn = checked;
        LOG1(m_recordingIsOn);
        DisplayManager::instance()->setRecordingChecked(m_recordingIsOn);

        if(m_recordingIsOn){
            ui->pushButtonRecord->setEnabled(false);
            DisplayManager::instance()->setRecordingEnabled(false);
        }

        if(!m_recordingIsInitialized){
            m_recordingIsInitialized = true;
            initRecording();
        }

        auto* recorder = OctFrameRecorder::instance();
        recorder->onRecordSector(m_recordingIsOn);
        if(m_recordingIsOn){
            int delay = userSettings::Instance().getRecordingDurationMin();
            const QString playListThumbnail(clipListModel::Instance().getPlaylistThumbnail());
            LOG1(playListThumbnail)
            QTimer::singleShot(delay, this, &MainScreen::enableRecordButton);
            m_scene->captureClip(playListThumbnail);

            // record the start time
            auto clipTimestamp = QDateTime::currentDateTime().toUTC();
            deviceSettings &dev = deviceSettings::Instance();
            clipListModel &clipList = clipListModel::Instance();
            clipList.addClipCapture( playListThumbnail,
                                     clipTimestamp.toTime_t(),
                                     clipListModel::Instance().getThumbnailDir(),
                                     dev.current()->getDeviceName(),
                                     true );

            recorder->start();
        }
        showYellowBorderForRecordingOn(m_recordingIsOn);
    }
}

void MainScreen::onCaptureImage()
{
    static int currentImageNumber = 0;
    // tag the images as "IMG1, IMG2, ..."
    currentImageNumber++;
    QString fileName = QString( "%1%2" ).arg( ImagePrefix ).arg( currentImageNumber);
    grabImage();
    m_scene->captureDecoratedImage(m_sectorImage, fileName);
}

void MainScreen::grabImage()
{
    QRect rectangle = m_graphicsView->rect();
//    LOG2( rectangle.width(),  rectangle.height())
    m_sectorImage = m_graphicsView->grab(rectangle).toImage();
}

void MainScreen::setMeasurementMode(bool enable)
{
    if( enable )
    {
        m_scene->setMeasureModeArea( true, Qt::magenta );
        setSceneCursor( QCursor( Qt::CrossCursor ) );
        ui->pushButtonMeasure->setChecked( true );
        LOG( INFO, "Measure Mode: start" )
    }
    else
    {
        m_scene->setMeasureModeArea( false, Qt::magenta );
        setSceneCursor( QCursor( Qt::OpenHandCursor ) );
        ui->pushButtonMeasure->setChecked( false );
        LOG( INFO, "Measure Mode: stop" )
    }
}

void MainScreen::enableRecordButton()
{
    ui->pushButtonRecord->setEnabled(true);
    DisplayManager::instance()->setRecordingEnabled(true);
}

void MainScreen::setSceneCursor( QCursor cursor )
{
    ui->graphicsView->viewport()->setProperty( "cursor", QVariant( cursor ) );
}

void MainScreen::updateSector(OCTFile::OctData_t *frameData)
{
    static uint32_t lastGoodImage = 0;
    static uint32_t missedImageCountAcc = 0;
    static int count{0};

    if(!m_scanWorker){
        m_scanWorker = new ScanConversion();
        m_imageDecimation = userSettings::Instance().getImageIndexDecimation();
        m_disableRendering = userSettings::Instance().getDisableRendering();
    }

    if(!frameData){
       auto* sm = SignalModel::instance();
       auto val = sm->getFromImageRenderingQueue();
       if(val.first)
       {
           auto& frame = val.second;
           int32_t missedImageCount = frame.frameCount - lastGoodImage - 1;
           if(lastGoodImage && (lastGoodImage < frame.frameCount) && (missedImageCount > 0) ){
                missedImageCountAcc += missedImageCount;
           }
           lastGoodImage = frame.frameCount;
           if(m_imageDecimation && (++count % m_imageDecimation == 0)){
               float percent = 100.0f * missedImageCountAcc / frame.frameCount;
               LOG4(frame.acqData, frame.frameCount, missedImageCountAcc, percent);
           }
           if(m_scene)
           {
               QImage* image = m_scene->sectorImage();

               frame.dispData = image->bits();
               auto bufferLength = frame.bufferLength;

               m_scanWorker->warpData( &frame, bufferLength);

               if(m_scanWorker->isReady){

                   if(image && frame.dispData){


                        updateMainScreenLabels(frame);

                       QGraphicsPixmapItem* pixmap = m_scene->sectorHandle();

                       if(pixmap && !m_disableRendering){
                           const QPixmap& tmpPixmap = QPixmap::fromImage( *image, Qt::MonoOnly);
                           pixmap->setPixmap(tmpPixmap);
                           if( userSettings::Instance().getIsRecording()){
                                ui->labelSim->setText(QString("recording ") + QString::number(frame.frameCount));
                           } else {
                                ui->labelSim->setText(QString("retrieving ") + QString::number(frame.frameCount));
                           }
                       }
                   }
               }
            }
        }
    }
}

void MainScreen::updateImage()
{
    updateSector(nullptr);
}

void MainScreen::on_pushButton_clicked()
{
    auto interfaceSupport = InterfaceSupport::getInstance();
    int currentSledRunningStateVal{interfaceSupport->getRunningState()};

    if (currentSledRunningStateVal == 0) {
        interfaceSupport->setSledRunState(true);
        ui->pushButton->setText("SledOn");
    } else if (currentSledRunningStateVal > 0) {
        interfaceSupport->setSledRunState(false);
        ui->pushButton->setText("SledOff");
    }
}

void MainScreen::initRecording()
{
    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    connect( this, &MainScreen::updateRecorder, OctFrameRecorder::instance(), &OctFrameRecorder::recordData);

}

