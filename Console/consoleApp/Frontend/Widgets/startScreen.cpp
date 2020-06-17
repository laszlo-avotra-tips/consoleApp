#include "startScreen.h"
#include "ui_startScreen.h"
#include "Utility/widgetcontainer.h"
#include "Widgets/caseinfowizardpage.h"
#include "Widgets/caseinfowizard.h"
#include <backend.h>
#include "devicewizard.h"
#include "logger.h"
#include "mainScreen.h"
#include "Frontend/Screens/frontend.h"
#include <daqfactory.h>
#include "deviceSettings.h"
#include "util.h"
#include "caseInformationDialog.h"
#include "deviceSelectDialog.h"

#include <QDebug>

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

void StartScreen::on_pushButtonStart_clicked()
{
    WidgetContainer::instance()->gotoScreen("l250Frontend");
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
