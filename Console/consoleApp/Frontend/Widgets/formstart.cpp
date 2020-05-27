#include "formstart.h"
#include "ui_formstart.h"
#include "Utility/widgetcontainer.h"
#include "Widgets/caseinfowizardpage.h"
#include "Widgets/caseinfowizard.h"
#include "logger.h"

#include <QDebug>

FormStart::FormStart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormStart)
{
    ui->setupUi(this);
    on_pushButtonMenu_clicked(ui->pushButtonMenu->isChecked());

    const int middleFrameWidth = WidgetContainer::instance()->middleFrameWidth();
    const int sideFrameWidth = int(middleFrameWidth * 0.25 );
    const int frameHeight = middleFrameWidth;
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

    ui->pushButtonStart->setIconSize(QSize(middleFrameWidth,middleFrameWidth));
    ui->pushButtonMenu->setIconSize(QSize(windowWidth/16, windowHeight/16));
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
//    m_backend.setupCase(true);
//    WidgetContainer::instance()->gotoPage("mainPage");
    caseInfoWizard *caseWizardLocal = new caseInfoWizard( this );

    // reload data for updating
    caseWizardLocal->init( caseInfoWizard::UpdateCaseSetup );
    // Force the wizard to the center of the primary monitor
    int x = ( 3240 / WidgetContainer::instance()->ratio() - caseWizardLocal->width() ) / 2;
    int y = ( 2160 / WidgetContainer::instance()->ratio() - caseWizardLocal->width() ) / 2;
    caseWizardLocal->setGeometry( x, y, caseWizardLocal->width(), caseWizardLocal->height() );
    int result = caseWizardLocal->exec();
    LOG1(result);
}
