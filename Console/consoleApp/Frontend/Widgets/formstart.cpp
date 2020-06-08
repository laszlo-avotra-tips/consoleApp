#include "formstart.h"
#include "ui_formstart.h"
#include "Utility/widgetcontainer.h"
#include "Widgets/caseinfowizardpage.h"
#include "Widgets/caseinfowizard.h"
#include <backend.h>
#include "devicewizard.h"
#include "logger.h"
#include "mainwindow.h"
#include "Frontend/Screens/frontend.h"
#include <daqfactory.h>
#include "deviceSettings.h"
#include "util.h"
#include "dialogcaseinformation.h"
#include "dialogdeviceselect.h"

#include <QDebug>

FormStart::FormStart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormStart)
{
    ui->setupUi(this);
    on_pushButtonMenu_clicked(ui->pushButtonMenu->isChecked());

    QString brandVersion("<html><head/><body><p><span style=\" font-size:42pt; font-weight:600; color:#A9A9A9;\">LIGHTBOX</span><span \
style=\" font-size:42pt;color:#A9A9A9;\"> L300 | Software Version ");
    ui->labelBrand->setText(brandVersion + getSoftwareVersionNumber() + QString("</span></p></body></html>"));

    QString service("<html><head/><body><p align=\"right\"><span style=\" font-size:28pt; font-weight:600;color:#A9A9A9;\">FOR SUPPORT, CALL CUSTOMER SERVICE AT 650-241-7900 \
</span></p><p align=\"right\"><span style=\" font-size:32pt; font-weight:600; color:#A9A9A9;\">2011-2020 AVINGER, INC.</span></p></body></html>");
    ui->labelService->setText(service);

    const int middleFrameWidth = WidgetContainer::instance()->middleFrameWidth();
    const int sideFrameWidth = int(middleFrameWidth * 0.25 );
    const int frameHeight = middleFrameWidth - 160;
    const bool isFullScreen = WidgetContainer::instance()->isFullScreen();

    qDebug() << "sideFrameWidth = " << sideFrameWidth << ", frameHeight = " << frameHeight;

    int windowWidth{3240};
    int windowHeight{2160};
    if(!isFullScreen){
        windowWidth = middleFrameWidth * 1.5;
        windowHeight = middleFrameWidth;
        setMinimumSize(windowWidth, windowHeight);
        setMaximumSize(windowWidth, windowHeight);
        qDebug() << "windowWidth = " << windowWidth << ", windowHeight = " << windowHeight;
    }

    ui->frameL->setMinimumSize(sideFrameWidth, frameHeight);
    ui->frameL->setMaximumSize(sideFrameWidth, frameHeight);

    ui->frameR->setMinimumSize(sideFrameWidth, frameHeight);
    ui->frameR->setMaximumSize(sideFrameWidth, frameHeight);

    ui->pushButtonStart->setIconSize(QSize(middleFrameWidth,frameHeight));
    ui->pushButtonMenu->setIconSize(QSize(windowWidth/16, windowHeight/16));

    m_backend = new Backend(parent);
}

FormStart::~FormStart()
{
    delete ui;
}

void FormStart::on_pushButtonMenu_clicked(bool checked)
{
    if(checked){
         ui->pushButtonExport->show();
         ui->pushButtonShutdown->show();
         ui->pushButtonReviewCase->show();
         ui->pushButtonPreferences->show();
    } else {
         ui->pushButtonExport->hide();
         ui->pushButtonShutdown->hide();
         ui->pushButtonReviewCase->hide();
         ui->pushButtonPreferences->hide();
    }
}

void FormStart::on_pushButtonPreferences_clicked()
{
}

void FormStart::on_pushButtonShutdown_clicked()
{
    WidgetContainer::instance()->close();
}

void FormStart::on_pushButtonStart_clicked()
{
//    int result{-1};
//    result = showCaseInfoDialog();
//    if(result == QDialog::Accepted){
//       result = showDeviceWizard();
//       if(result == QDialog::Accepted){
           auto widget = WidgetContainer::instance()->gotoPage("frontendPage");
//           MainWindow* mw = dynamic_cast<MainWindow*>(widget);
//           if(mw){
//               mw->setDeviceLabel();
//           }
//           frontend* fw = dynamic_cast<frontend*>(widget);
//           if(fw){
//              fw->showFullScreen();
//              fw->updateDeviceLabel();
//              startDaq(fw);
//           }
//       }else {
//           WidgetContainer::instance()->gotoPage("startPage");
//       }
           //    }
}

void FormStart::showEvent(QShowEvent *se)
{
    QWidget::showEvent( se );
    qDebug() << __FUNCTION__;
    WidgetContainer::instance()->setIsNewCase(true);
}

void FormStart::hideEvent(QHideEvent *he)
{
    QWidget::hideEvent( he );
}

int FormStart::showCaseInfoDialog()
{
//    caseInfoWizard *caseWizardLocal = new caseInfoWizard( this );

//    // reload data for updating
//    caseWizardLocal->init( caseInfoWizard::UpdateCaseSetup );
//    int result = caseWizardLocal->exec();
    DialogCaseInformation* caseInfo = new DialogCaseInformation(this);


    int result = caseInfo->exec();

    LOG1(result)
    return result;
}

int FormStart::showDeviceWizard()
{
//    deviceWizard* device = new deviceWizard(this);

//    int result = device->exec();
    DialogDeviceSelect* deviceSelect = new DialogDeviceSelect(this);

    int result = deviceSelect->exec();

    LOG1(result)
    return result;
}

void FormStart::startDaq(frontend *fe)
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
