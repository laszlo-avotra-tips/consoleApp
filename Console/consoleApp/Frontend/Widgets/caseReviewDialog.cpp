#include "caseReviewDialog.h"
#include "ui_caseReviewDialog.h"
#include "logger.h"


CaseReviewDialog::CaseReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseReviewDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

//    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
//    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    LOG1("construct case review")
    m_player = new VideoPlayer(this);
    m_player->setVideoWidgetContainer(ui->verticalLayout);
    m_player->init();

}

CaseReviewDialog::~CaseReviewDialog()
{
    LOG1("destruct case review")
    delete ui;
}

void CaseReviewDialog::on_pushButtonBack_clicked()
{
    delete m_player;
    accept();
}
