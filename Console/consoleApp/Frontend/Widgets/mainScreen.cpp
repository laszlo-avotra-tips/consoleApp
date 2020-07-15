#include "mainScreen.h"
#include "ui_mainScreen.h"
#include "Utility/widgetcontainer.h"
#include "Utility/screenFactory.h"
#include "Frontend/Screens/frontend.h"
#include "devicewizard.h"
#include "deviceselectwizardpage.h"
#include "deviceSettings.h"
#include "daqfactory.h"
#include <logger.h>
#include "opacScreen.h"
#include "Frontend/Screens/frontend.h"
#include "Frontend/Widgets/caseInformationDialog.h"
#include "sledsupport.h"
#include <QTimer>


#include <QDebug>
#include <QLayoutItem>
#include <QLayout>
#include <QFile>
#include <QTextStream>

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

    auto wid = WidgetContainer::instance()->getScreen("l250Frontend");

    frontend* fw = dynamic_cast<frontend*>(wid);
    if(fw)
    {
        m_frontEndWindow = fw;
    }

    m_updatetimeTimer.start(500);
    connect(&m_updatetimeTimer, &QTimer::timeout, this, &MainScreen::updateTime);

    m_opacScreen = new OpacScreen(this);
    m_opacScreen->show();
    m_graphicsView->hide();
    ui->frameSpeed->hide();

    connect(ui->pushButtonLow, &QPushButton::clicked, this, &MainScreen::udpateToSpeed1);
    connect(ui->pushButtonMedium, &QPushButton::clicked, this, &MainScreen::udpateToSpeed2);
    connect(ui->pushButtonHigh, &QPushButton::clicked, this, &MainScreen::udpateToSpeed3);
    connect(this, &MainScreen::sledRunningStateChanged, this, &MainScreen::handleSledRunningStateChanged);
}

void MainScreen::setScene(liveScene *scene)
{
    if(!m_scene){
        m_scene = scene;
        m_graphicsView->setScene(m_scene);
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

void MainScreen::setSpeed(int speed)
{
    LOG1(speed);
    const QString qSpeed(QString::number(speed));
    const QByteArray baSpeed(qSpeed.toStdString().c_str());
    SledSupport::Instance().setSledSpeed(baSpeed);

}

void MainScreen::highlightSpeedButton(QPushButton *wid)
{
    ui->pushButtonLow->setStyleSheet("");
    ui->pushButtonMedium->setStyleSheet("");
    ui->pushButtonHigh->setStyleSheet("");

    wid->setStyleSheet("background-color: #F5C400; color: black;");
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
    m_opacScreen->show();
    m_graphicsView->hide();

    CaseInformationDialog::reset();

    WidgetContainer::instance()->gotoScreen("startScreen");

    WidgetContainer::instance()->unRegisterWidget("l2500Frontend");

    m_updatetimeTimer.stop();
    ui->labelRunTime->setText(QString("Runtime: 00:00"));
    ui->frameSpeed->hide();
}

void MainScreen::on_pushButtonDownArrow_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainScreen::on_pushButtonCondensUp_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainScreen::resetYellowBorder()
{
    ui->graphicsView->setStyleSheet("border:5px solid rgb(0,0,0);");
}

void MainScreen::setDeviceLabel()
{
    ui->labelDevice->show();
    ui->labelRunTime->show();

    deviceSettings &dev = deviceSettings::Instance();
    const QString name{dev.getCurrentDeviceTitle()};
    ui->labelDevice->setText(name);
    m_opacScreen->hide();
    m_graphicsView->show();
    m_runTime.start();
    m_updatetimeTimer.start(500);
    updateTime();
    udpateToSpeed2();
}

void MainScreen::showSpeed(bool isShown)
{
    ui->frameSpeed->setVisible(isShown);
//    ui->labelLive->setStyleSheet("color: green;");
}

void MainScreen::on_pushButtonSettings_clicked()
{
    auto result = WidgetContainer::instance()->openDialog(this, "reviewAndSettingsDialog");
    if(result.first){
        result.first->hide();
    }
    if( result.second == QDialog::Accepted){
        qDebug() << "Accepted";
    }
    else {
        qDebug() << "Cancelled";
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

void MainScreen::openDeviceSelectDialog()
{
    auto result = WidgetContainer::instance()->openDialog(this,"deviceSelectDialog");

    if( result.second == QDialog::Accepted){
        qDebug() << "Accepted";
    } else {
        qDebug() << "Cancelled";
        openCaseInformationDialog();
    }
}

void MainScreen::updateTime()
{
    int ms{0};

    updateSledRunningState();

    if(m_runTime.isValid()){
        ms = m_runTime.elapsed();
    }

    if(ms){
        int durationInSec = ms / 1000;
        int sec = durationInSec % 60;
        int min = durationInSec / 60;
        QTime dt(0,min,sec,0);

        QString elapsed = dt.toString("mm:ss");
        if(elapsed.isEmpty() && !m_runTime.isValid()){
             ui->labelRunTime->setText(QString("Runtime: 00:00"));
        }else {
            ui->labelRunTime->setText(QString("Runtime: ") + elapsed);
        }
    }

    setCurrentTime();
}

void MainScreen::udpateToSpeed1()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin1();

    setSpeed(speed);
    highlightSpeedButton(ui->pushButtonLow);
}

void MainScreen::udpateToSpeed2()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin2();

    setSpeed(speed);
    highlightSpeedButton(ui->pushButtonMedium);
}

void MainScreen::udpateToSpeed3()
{
    deviceSettings& ds = deviceSettings::Instance();
    auto* cd = ds.current();
    auto speed = cd->getRevolutionsPerMin3();

    setSpeed(speed);
    highlightSpeedButton(ui->pushButtonHigh);
}

void MainScreen::on_pushButtonCapture_released()
{
    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QString yellowBorder("border:5px solid rgb(245,196,0);");
    ui->graphicsView->setStyleSheet(yellowBorder);
    emit captureImage();
    QTimer::singleShot(500,this,&MainScreen::resetYellowBorder);
}

void MainScreen::on_pushButtonMeasure_clicked(bool checked)
{
    emit measureImage(checked);
}

void MainScreen::updateSledRunningState()
{
    bool currentSledRunningState{SledSupport::Instance().isRunningState()};

     if(m_sledIsInRunningState != currentSledRunningState){
         m_sledIsInRunningState = currentSledRunningState;
         emit sledRunningStateChanged(m_sledIsInRunningState);
     }
}

void MainScreen::handleSledRunningStateChanged(bool isInRunningState)
{
    LOG1(isInRunningState);

    if(isInRunningState){
        ui->labelLive->setStyleSheet("color: green;");
    }else{
        ui->labelLive->setStyleSheet("color: grey;");
    }
    //exit while in measure mode and the sled is started
    if(!m_sledIsInRunningState && ui->pushButtonMeasure->isChecked()){
        ui->pushButtonMeasure->setChecked(false);
    }

    if(m_sledIsInRunningState && ui->pushButtonMeasure->isChecked()){
        emit measureImage(false);
    }

    ui->pushButtonMeasure->setEnabled(!m_sledIsInRunningState);
}

void MainScreen::on_pushButtonRecord_clicked()
{
    hide();
}
