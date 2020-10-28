#include "caseReviewDialog.h"
#include "ui_caseReviewDialog.h"

CaseReviewDialog::CaseReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseReviewDialog)
{
    ui->setupUi(this);
}

CaseReviewDialog::~CaseReviewDialog()
{
    delete ui;
}
