#include "caseReviewDialog.h"
#include "ui_caseReviewDialog.h"

CaseReviewDialog::CaseReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseReviewDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );
}

CaseReviewDialog::~CaseReviewDialog()
{
    delete ui;
}
