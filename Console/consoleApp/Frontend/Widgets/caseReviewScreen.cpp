#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"
#include "Utility/widgetcontainer.h"

CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
    m_player = new VideoPlayer(this);
    m_player->setVideoWidgetContainer(ui->verticalLayout);
    m_player->init();
}

CaseReviewScreen::~CaseReviewScreen()
{
    delete m_player;
    delete ui;
}

void CaseReviewScreen::on_pushButtonBack_clicked()
{
    WidgetContainer::instance()->gotoScreen("mainScreen");
}
