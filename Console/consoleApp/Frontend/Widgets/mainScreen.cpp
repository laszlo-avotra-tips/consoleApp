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
#include "sledsupport.h"
#include "livescene.h"
#include "scanconversion.h"
#include "signalmodel.h"
#include "daqfactory.h"
#include "idaq.h"
#include "Utility/userSettings.h"
#include "Utility/clipListModel.h"

#include <QTimer>
#include <QDebug>
#include <QLayoutItem>
#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QGraphicsView>
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

    m_updatetimeTimer.start(500);
    connect(&m_updatetimeTimer, &QTimer::timeout, this, &MainScreen::updateTime);

    m_opacScreen = new OpaqueScreen(this);
    m_opacScreen->show();
    m_graphicsView->hide();
    ui->frameSpeed->hide();

    connect(ui->pushButtonLow, &QPushButton::clicked, this, &MainScreen::udpateToSpeed1);
    connect(ui->pushButtonMedium, &QPushButton::clicked, this, &MainScreen::udpateToSpeed2);
    connect(ui->pushButtonHigh, &QPushButton::clicked, this, &MainScreen::udpateToSpeed3);
    connect(this, &MainScreen::sledRunningStateChanged, this, &MainScreen::handleSledRunningState);

    const double scaleUp = 2.1; //lcv zomFactor
    QMatrix matrix = ui->graphicsView->matrix();
    ui->graphicsView->setTransform( QTransform::fromScale( scaleUp * matrix.m11(), scaleUp * matrix.m22() ) );

    m_scene = new liveScene( this );
    m_graphicsView->setScene(m_scene);
    m_scene->handleReticleBrightnessChanged();

    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void MainScreen::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
        daqfactory::instance()->getdaq();
    }
}

MainScreen::~MainScreen()
{
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
}

void MainScreen::setSpeedAndEnableDisableBidirectional(int speed)
{
    if(speed >= 600){
        LOG1(speed);

        auto idaq = daqfactory::instance()->getdaq();
        if(idaq){
            idaq->setSubsampling(speed);
        }

        const QString qSpeed(QString::number(speed));
        const QByteArray baSpeed(qSpeed.toStdString().c_str());
        auto& sled = SledSupport::Instance();
        sled.setSledSpeed(baSpeed);
        QThread::msleep(200);
        sled.enableDisableBidirectional();
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
    if(m_recordingIsOn){
        LOG1(m_recordingIsOn)
        emit on_pushButtonRecord_clicked(false);
    }

    QTimer::singleShot(1000, [this](){
        m_opacScreen->show();
        m_graphicsView->hide();

        CaseInformationDialog::reset();

        WidgetContainer::instance()->gotoScreen("startScreen");

        WidgetContainer::instance()->unRegisterWidget("l2500Frontend");

        m_sledRuntime = 0;
        m_runTime.invalidate();

        m_updatetimeTimer.stop();
        ui->labelRunTime->setText(QString("Runtime: 0:00:00"));
        ui->frameSpeed->hide();

        captureListModel::Instance().reset();
        LOG1(m_recordingIsOn)
    });
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
    if(m_recordingIsOn){
        ui->graphicsView->setStyleSheet("border:1px solid rgb(245,196,0);");
    } else {
        ui->graphicsView->setStyleSheet("border:5px solid rgb(0,0,0);");
    }
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
    m_updatetimeTimer.start(500);
    updateTime();
}

void MainScreen::showSpeed(bool isShown)
{
    ui->frameSpeed->setVisible(isShown);
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
        QTimer::singleShot(100,this, &MainScreen::openCaseInformationDialog);
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

    if(result.first){
        result.first->hide();
    }
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
    const bool isBidir = selectedDevice->isBiDirectional();
    const int numberOfSpeeds = selectedDevice->getNumberOfSpeeds();

    auto& sled = SledSupport::Instance();
    int currentSledRunningStateVal{sled.runningState()};
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
    if(m_recordingIsOn){
        QString yellowBorder("border:1px solid rgb(245,196,0);");
        ui->graphicsView->setStyleSheet(yellowBorder);
    } else {
        QString noBorder("border:0px solid rgb(0,0,0);");
        ui->graphicsView->setStyleSheet(noBorder);
    }
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
    }
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
        }else {
            ui->labelRunTime->setText(QString("Runtime: ") + runtimeDisplay);
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
}

void MainScreen::udpateToSpeed2()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin2();

    setSpeedAndEnableDisableBidirectional(speed);
    highlightSpeedButton(ui->pushButtonMedium);
}

void MainScreen::udpateToSpeed3()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin3();

    setSpeedAndEnableDisableBidirectional(speed);
    highlightSpeedButton(ui->pushButtonHigh);
}

void MainScreen::on_pushButtonCapture_released()
{
    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    QString blackBorder("border:5px solid rgb(0,0,0);");
    ui->graphicsView->setStyleSheet(blackBorder);

    onCaptureImage();

    QString yellowBorder("border:5px solid rgb(245,196,0);");
    ui->graphicsView->setStyleSheet(yellowBorder);
    QTimer::singleShot(500,this,&MainScreen::handleYellowBorder);
}

void MainScreen::on_pushButtonMeasure_clicked(bool checked)
{
//    emit measureImage(checked);
    setMeasurementMode(checked);
}

void MainScreen::updateSledRunningState()
{
    int currentSledRunningStateVal{SledSupport::Instance().runningState()};

     if(m_sledRunningStateVal != currentSledRunningStateVal)
     {
         m_sledRunningStateVal = currentSledRunningStateVal;
         emit sledRunningStateChanged(m_sledRunningStateVal);
     }
}

void MainScreen::handleSledRunningState(int runningStateVal)
{
    m_sledIsInRunningState = (runningStateVal == 1) || (runningStateVal == 3);

    auto&ds = deviceSettings::Instance();
    auto device = ds.current();

    if(device && m_scene){
        const bool isBd = device->isBiDirectional();

        if(runningStateVal == 1){
            ui->labelLive->setStyleSheet("color: green;");
            if(isBd){
                m_scene->setActive();
            }
        } else if (runningStateVal == 3){
            ui->labelLive->setStyleSheet("color: green;");
            if(isBd){
                m_scene->setPassive();
            }
        }else{
            ui->labelLive->setStyleSheet("color: grey;");
            m_scene->setIdle();
        }
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
        LOG1(m_recordingIsOn)

        if(!m_recordingIsInitialized){
            m_recordingIsInitialized = true;
            initRecording();
        }

        auto* recorder = OctFrameRecorder::instance();
        recorder->onRecordSector(m_recordingIsOn);
        if(m_recordingIsOn){
//            recorder->onRecordSector(m_recordingIsOn);
            ui->pushButtonRecord->setEnabled(false);
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

        }

        showYellowBorderForRecordingOn(m_recordingIsOn);
    }
}


void MainScreen::on_pushButtonRecord_clicked()
{
//    if(!m_recordingIsInitialized){
//        m_recordingIsInitialized = true;
//        initRecording();
//    }

//    auto* recorder = OctFrameRecorder::instance();
//    if(m_recordingIsOn){
//        m_recordingIsOn = false;
//        recorder->onRecordSector(m_recordingIsOn);
//    } else {
//        m_recordingIsOn = true;
//        recorder->onRecordSector(m_recordingIsOn);
//        ui->pushButtonRecord->setEnabled(false);
//        int delay = userSettings::Instance().getRecordingDurationMin();
//        const QString playListThumbnail(clipListModel::Instance().getPlaylistThumbnail());
//        LOG1(playListThumbnail)
//        QTimer::singleShot(delay, this, &MainScreen::enableRecordButton);
//        m_scene->captureClip(playListThumbnail);

//        // record the start time
//        auto clipTimestamp = QDateTime::currentDateTime().toUTC();
//        deviceSettings &dev = deviceSettings::Instance();
//        clipListModel &clipList = clipListModel::Instance();
//        clipList.addClipCapture( playListThumbnail,
//                                 clipTimestamp.toTime_t(),
//                                 clipListModel::Instance().getThumbnailDir(),
//                                 dev.current()->getDeviceName(),
//                                 true );

//    }

//    showYellowBorderForRecordingOn(m_recordingIsOn);
}

void MainScreen::onCaptureImage()
{
    static int currentImageNumber = 0;
    // tag the images as "IMG1, IMG2, ..."
    currentImageNumber++;
    QString fileName = QString( "%1%2" ).arg( ImagePrefix ).arg( currentImageNumber);
    LOG1(fileName);
    QRect rectangle = ui->graphicsView->rect();
    qDebug() << __FUNCTION__ << ": width=" << rectangle.width() << ", height=" << rectangle.height();
    QImage p = ui->graphicsView->grab(rectangle).toImage();
    m_scene->captureDecoratedImage( p, fileName );

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
}

void MainScreen::setSceneCursor( QCursor cursor )
{
    ui->graphicsView->viewport()->setProperty( "cursor", QVariant( cursor ) );
}

void MainScreen::updateSector(OCTFile::OctData_t *frameData)
{
    static int count = -1;
    if(!m_scanWorker){
        m_scanWorker = new ScanConversion();
    }
    if(frameData && m_scene && m_scanWorker){

        const auto* sm =  SignalModel::instance();

        QImage* image = m_scene->sectorImage();

        frameData->dispData = image->bits();
        auto bufferLength = sm->getBufferLength();

        m_scanWorker->warpData( frameData, bufferLength);

        if(m_scanWorker->isReady){

            if(image && frameData && frameData->dispData){

                emit updateRecorder(frameData->dispData);

                QGraphicsPixmapItem* pixmap = m_scene->sectorHandle();

                if(pixmap){
                    QPixmap tmpPixmap = QPixmap::fromImage( *image, Qt::MonoOnly);
                    pixmap->setPixmap(tmpPixmap);
                }
//lcv                if(++count % 2 == 0)
                    m_scene->setDoPaint();
            }
        }
    }
}

void MainScreen::on_pushButton_clicked()
{
    static bool sledIsOn{false};

    auto& sled = SledSupport::Instance();

    if(sledIsOn){
        sledIsOn = false;
        sled.writeSerial("sr0\r");
        ui->pushButton->setText("SledOn");
    } else {
        sledIsOn = true;
        sled.writeSerial("sr1\r");
        ui->pushButton->setText("SledOff");
    }

}

void MainScreen::initRecording()
{
    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    connect( this, &MainScreen::updateRecorder, OctFrameRecorder::instance(), &OctFrameRecorder::recordData);

}
