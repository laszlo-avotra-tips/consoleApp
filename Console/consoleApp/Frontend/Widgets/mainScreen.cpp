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
//    m_graphicsView->setMaximumSize(m_sceneSize);
//    m_graphicsView->setMinimumSize(m_sceneSize);

//    const int h{getSceneWidth()};
//    const int ratio = WidgetContainer::instance()->ratio();
//    if(!ratio){
//        return;
//    }
//    const int height = m_sceneWidth / ratio;
//    const int wL = (3240 - 2160) / ratio;
//    const int wM = height;
//    const int wR = 3240 / ratio - wM - wL;
//    const QSize sL(wL,height);
//    const QSize sM(wM,height);
//    const QSize sR(wR,height);

//    ui->frameM->setMaximumSize(sM);
//    ui->frameM->setMinimumSize(sM);

//    ui->frameL->setMaximumSize(sL);
//    ui->frameL->setMinimumSize(sL);

//    ui->frameR->setMaximumSize(sR);
//    ui->frameR->setMinimumSize(sR);

    m_navigationButtons.push_back(ui->pushButtonEndCase);
    m_navigationButtons.push_back(ui->pushButtonSettings);
    m_navigationButtons.push_back(ui->pushButtonMeasure);
    m_navigationButtons.push_back(ui->pushButtonRecord);
    m_navigationButtons.push_back(ui->pushButtonCapture);
    m_navigationButtons.push_back(ui->pushButtonFlip);
//    for(auto* button : m_navigationButtons){
//        button->hide();
//    }
    ui->pushButtonDownArrow->hide();
    ui->pushButtonCondensUp->show();

//    ui->labelDevice->hide();
//    ui->labelRunTime->hide();

    auto wid = WidgetContainer::instance()->getPage("l250Frontend");

    frontend* fw = dynamic_cast<frontend*>(wid);
    if(fw)
    {
        m_frontEndWindow = fw;
    }

    m_updatetimeTimer.start(500);
    connect(&m_updatetimeTimer, &QTimer::timeout, this, &MainScreen::updateTime);
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
    WidgetContainer::instance()->gotoPage("startScreen");
}

void MainScreen::on_pushButtonDownArrow_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainScreen::on_pushButtonCondensUp_clicked()
{
    on_pushButtonMenu_clicked();
}

void MainScreen::on_pushButtonCapture_clicked()
{
}

void MainScreen::setDeviceLabel()
{
    ui->labelDevice->show();
    ui->labelRunTime->show();

    deviceSettings &dev = deviceSettings::Instance();
    const QString name{dev.getCurrentDeviceTitle()};
    ui->labelDevice->setText(name);
//    setCurrentTime();
    m_runTime.start();
    updateTime();
}

void MainScreen::showSpeed(bool isShown)
{
    ui->frameSpeed->setVisible(isShown);
    ui->labelLive->setStyleSheet("color: green;");
}

void MainScreen::on_pushButtonSettings_clicked()
{
    hide();
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
//        QTimer::singleShot(100,this, &MainScreen::openGreenDialog);
        openDeviceSelectDialog();
    }
    else {
        qDebug() << "Cancelled";
        WidgetContainer::instance()->gotoPage("startScreen");
    }
}

void MainScreen::openDeviceSelectDialog()
{
    auto result = WidgetContainer::instance()->openDialog(this,"deviceSelectDialog");

    if( result.second == QDialog::Accepted){
        qDebug() << "Accepted";
    } else {
        qDebug() << "Cancelled";
//        QTimer::singleShot(100,this, &MainScreen::openMainWindowDialog);
        openCaseInformationDialog();
    }
}

void MainScreen::updateTime()
{
    int ms = m_runTime.elapsed();

    if(ms){
        int durationInSec = ms / 1000;
        int sec = durationInSec % 60;
        int min = durationInSec / 60;
        QTime dt(0,min,sec,0);

        QString elapsed = dt.toString("mm:ss");
        if(elapsed.isEmpty()){
             ui->labelRunTime->setText(QString("Runtime: 00:00"));
        }else {
            ui->labelRunTime->setText(QString("Runtime: ") + elapsed);
        }
    }

   setCurrentTime();
}
