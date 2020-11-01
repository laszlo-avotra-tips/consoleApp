#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"

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
