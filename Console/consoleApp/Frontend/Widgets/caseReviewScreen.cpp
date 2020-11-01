#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"
#include "Utility/widgetcontainer.h"

CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
}

CaseReviewScreen::~CaseReviewScreen()
{
    delete ui;
}

void CaseReviewScreen::on_pushButtonBack_clicked()
{
    WidgetContainer::instance()->gotoScreen("mainScreen");
}
