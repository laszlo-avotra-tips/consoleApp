#include "startScreen.h"
#include "ui_startScreen.h"
#include "Utility/widgetcontainer.h"
#include <backend.h>
#include "devicewizard.h"
#include "logger.h"
#include "mainScreen.h"
#include "Frontend/Screens/frontend.h"
#include <daqfactory.h>
#include "deviceSettings.h"
#include "util.h"
#include "idaq.h"

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

    QString brandVersion("<html><head/><body><p><span style=\" font-size:21pt; font-weight:600; color:#A9A9A9;\">LIGHTBOX</span><span \
style=\" font-size:21pt;color:#A9A9A9;\"> L300 | Software Version ");
    ui->labelBrand->setText(brandVersion + getSoftwareVersionNumber() + QString("</span></p></body></html>"));

    QString service("<html><head/><body><p align=\"right\"><span style=\" font-size:14pt; font-weight:600;color:#A9A9A9;\">FOR SUPPORT, CALL CUSTOMER SERVICE AT 650-241-7900 </span></p></body></html>");

    QString copyright("<html><head/><body><p align=\"right\"><span style=\" font-size:14pt; font-weight:600; color:#A9A9A9;\"> 2011-2020 AVINGER, INC.</span></p></body></html>");

    ui->labelService->setText(service);
    ui->labelCopyright->setText(copyright);


    const int sideFrameWidth = 640;
    const int middleFrameWidth = 3240 - 2 * sideFrameWidth;
    const int middleFrameHight = 2160;
    const int frameHeight = middleFrameHight - 160;

    const int windowWidth{3240};
    const int windowHeight{2160};

    setMinimumSize(windowWidth, windowHeight);
    setMaximumSize(windowWidth, windowHeight);

    ui->frameL->setMinimumSize(sideFrameWidth, frameHeight);
    ui->frameL->setMaximumSize(sideFrameWidth, frameHeight);

    ui->frameR->setMinimumSize(sideFrameWidth, frameHeight);
    ui->frameR->setMaximumSize(sideFrameWidth, frameHeight);

    ui->pushButtonStart->setIconSize(QSize(middleFrameWidth,middleFrameWidth));
    ui->pushButtonMenu->setIconSize(QSize(windowWidth/16, windowHeight/16));

    m_backend = new Backend(parent);

    grabGesture(Qt::TapAndHoldGesture);
    QTapAndHoldGesture::setTimeout(2000);
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
}

void StartScreen::on_pushButtonShutdown_clicked()
{
    WidgetContainer::instance()->close();
}

void StartScreen::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    qDebug() << __FUNCTION__;
    WidgetContainer::instance()->setIsNewCase(true);
}

void StartScreen::hideEvent(QHideEvent *he)
{
    QWidget::hideEvent( he );
}

void StartScreen::startDaq(frontend *fe)
{
    auto idaq = daqfactory::instance()->getdaq();

    if(!idaq){
        fe->abortStartUp();

        LOG( INFO, "Device not supported. OCT Console cancelled" )
    }
    fe->setIDAQ(idaq);
    LOG( INFO, "LASER: serial port control is DISABLED" )
    LOG( INFO, "SLED support board: serial port control is DISABLED" )

    fe->startDaq();
    auto& setting = deviceSettings::Instance();
    if(setting.getIsSimulation()){
        fe->startDataCapture();
    }
    fe->on_zoomSlider_valueChanged(100);
}

void StartScreen::on_pushButtonStart_released()
{
    if(!m_isPressAndHold){
//        m_timer.stop();
        WidgetContainer::instance()->gotoScreen("l250Frontend");
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
}
