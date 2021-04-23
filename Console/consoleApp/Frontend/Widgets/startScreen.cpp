#include "startScreen.h"
#include "ui_startScreen.h"
#include "Utility/widgetcontainer.h"
#include <backend.h>
#include "logger.h"
#include "mainScreen.h"
#include "deviceSettings.h"
#include "util.h"
#include "idaq.h"
#include "fullCaseRecorder.h"
#include "displayManager.h"
#include "daqfactory.h"
#include "preferencesDialog.h"
#include "shutdownConfirmationDialog.h"
#include "Utility/screenFactory.h"

#include <QDebug>
#include <QTimer>
#include <QTapAndHoldGesture>
#include <QGestureEvent>

StartScreen::StartScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartScreen)
{
    ui->setupUi(this);
    on_pushButtonMenu_clicked(ui->pushButtonMenu->isChecked());

    m_preferencesDialog = new PreferencesDialog(this);

    QString brandVersion("<html><head/><body><p><span style=\" font-size:21pt; font-weight:600; color:#A9A9A9;\">LIGHTBOX</span><span \
style=\" font-size:21pt;color:#A9A9A9;\"> L300 | Software Version ");
    ui->labelBrand->setText(brandVersion + getSoftwareVersionNumber() + QString("</span></p></body></html>"));

    QString service("<html><head/><body><p align=\"right\"><span style=\" font-size:14pt; font-weight:600;color:#A9A9A9;\">FOR SUPPORT, CALL CUSTOMER SERVICE AT 650-241-7900 </span></p></body></html>");

    QString copyright("<html><head/><body><p align=\"right\"><span style=\" font-size:14pt; font-weight:600; color:#A9A9A9;\"> 2011-2020 AVINGER, INC.</span></p></body></html>");

    ui->labelService->setText(service);
    ui->labelCopyright->setText(copyright);


    const int sideFrameWidth = 640;
    const int middleFrameWidth = ControlScreenWidth - 2 * sideFrameWidth;
    const int middleFrameHight = ControlScreenHeight;
    const int frameHeight = middleFrameHight - 160;

    const int windowWidth{ControlScreenWidth};
    const int windowHeight{ControlScreenHeight};

    setMinimumSize(windowWidth, windowHeight);
    setMaximumSize(windowWidth, windowHeight);

    ui->frameL->setMinimumSize(sideFrameWidth, frameHeight);
    ui->frameL->setMaximumSize(sideFrameWidth, frameHeight);

    ui->frameR->setMinimumSize(sideFrameWidth, frameHeight);
    ui->frameR->setMaximumSize(sideFrameWidth, frameHeight);

    ui->pushButtonStart->setIconSize(QSize(middleFrameWidth,middleFrameWidth));
    ui->pushButtonMenu->setIconSize(QSize(windowWidth/16, windowHeight/16));

//    m_backend = new Backend(parent);

    grabGesture(Qt::TapAndHoldGesture);
    QTapAndHoldGesture::setTimeout(2000);

     ui->pushButtonDemoMode->hide();

     DisplayManager::instance();
//     DisplayManager::instance()->initWidgetForTheSecondMonitor("logo");
     hookupStartUpDiagnostics();
}

void StartScreen::hookupStartUpDiagnostics() {
    diagnostics = new StartUpDiagnostics();
    auto messageBox = styledMessageBox::instance(); //new PowerUpMessageBox();

    QObject::connect(diagnostics, &OctSystemDiagnostics::showMessageBox,
                     messageBox, &styledMessageBox::onShowMessageBox);
    QObject::connect(diagnostics, &OctSystemDiagnostics::hideMessageBox,
                     messageBox, &styledMessageBox::onHideMessageBox);

    QObject::connect(messageBox, &styledMessageBox::userAcknowledged,
                     diagnostics, &OctSystemDiagnostics::onUserAcknowledged);
    LOG(INFO, "Start Up diagnostics framework initialized");
}

StartScreen::~StartScreen()
{
    delete ui;
}

void StartScreen::on_pushButtonMenu_clicked(bool checked)
{
    if(checked){
         ui->pushButtonShutdown->show();
         ui->pushButtonCaseReviewAndExport->show();
         ui->pushButtonPreferences->show();
    } else {
         ui->pushButtonShutdown->hide();
         ui->pushButtonCaseReviewAndExport->hide();
         ui->pushButtonPreferences->hide();
    }
}

void StartScreen::on_pushButtonPreferences_clicked()
{
    if(m_preferencesDialog->exec() == QDialog::Accepted){
        LOG1("ACCEPTED");
        m_preferencesDialog->close();
    }
}

void StartScreen::on_pushButtonShutdown_clicked()
{
//    FullCaseRecorder::instance()->closeRecorder();

    auto dialog = new ShutdownConfirmationDialog();

    if(dialog->exec() == QDialog::Accepted){
        WidgetContainer::instance()->close();

        DisplayManager::instance()->killDisplayMonitor();

        auto idaq = daqfactory::instance()->getdaq();
        idaq->shutdownDaq();
        QThread::sleep(1);
        InterfaceSupport::releaseInstance();
        LOG( INFO, "FTDI interface closed successfully");
    } else {
        DisplayManager::instance()->showOnTheSecondMonitor("logo");
    }
    delete dialog;
}

void StartScreen::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    if(se->type() == QEvent::Show){
        DisplayManager::instance()->showOnTheSecondMonitor("logo");
        WidgetContainer::instance()->setIsNewCase(true);

        auto* ifs = InterfaceSupport::getInstance();
        ifs->turnOnOffACPowerToOCT(false);//1. sac0
        ifs->setVOAMode(false);//2. svb
        ifs->turnOnOffSled5V(false); // 3, OFF "sled 5v"
        ifs->turnOnOffSled24V(false); //3. OFF "sled 24v"

        LOG2(ifs->getSupplyVoltage(), ifs->getVOASettings());
    }
}

void StartScreen::hideEvent(QHideEvent *he)
{
    QWidget::hideEvent( he );
}

void StartScreen::on_pushButtonStart_released()
{
    if (diagnostics) {
        if (!diagnostics->performDiagnostics(true)) {
            LOG(ERROR, "Start up diagnostics failed!");
            return;
        } else {
            LOG(INFO, "Start up diagnostics succeeded!");
        }
    }

    if(!m_isPressAndHold){

        auto* ifs = InterfaceSupport::getInstance();
        ifs->turnOnOffACPowerToOCT(true);//1. sac1
        ifs->turnOnOffSled5V(true); // 3, ON "sled 5v"
        ifs->turnOnOffSled24V(true); //3. ON "sled 24v"
        ifs->setVOAMode(false);//2. svb
        LOG2(ifs->getSupplyVoltage(), ifs->getVOASettings());

        ScreenFactory sf;
        sf.unRegisterScreens();
        sf.registerScreens();

        WidgetContainer::instance()->gotoScreen("mainScreen");
    }
}

void StartScreen::setPressAndHold()
{
    m_isPressAndHold = true;
    WidgetContainer::instance()->minimize();
}

bool StartScreen::event(QEvent *event)
{
    if(event->type() == QEvent::Gesture){
        LOG1(event->type());

        QGestureEvent* ge = dynamic_cast<QGestureEvent*>(event);
        if(ge){
            return gestureEvent(ge);
        }
    }
    return QWidget::event(event);
}

bool StartScreen::gestureEvent(QGestureEvent *ge)
{
    bool isHandled{false};
    LOG1(ge->type());

    if (QGesture *th = ge->gesture(Qt::TapAndHoldGesture))  {

        QTapAndHoldGesture* qth = dynamic_cast<QTapAndHoldGesture*>(th);
        if(qth){
            auto x = qth->position().rx();
            auto y = qth->position().ry();
            const qreal ox = 800;
            const qreal oy = 2080;
            LOG2(x, y);
            if(abs(ox - x) < 800 && abs(oy - y) < 80){
                setPressAndHold();
                isHandled = true;
            }
        }
    }

    return isHandled;
}

void StartScreen::on_pushButtonStart_pressed()
{
    m_isPressAndHold = false;
//    m_timer.singleShot(2000, this, &StartScreen::setPressAndHold);
//    DisplayManager::instance()->initWidgetForTheSecondMonitor("disk");
    m_backend = new Backend(this);
}
