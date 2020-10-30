#include "caseReviewDialog.h"
#include "ui_caseReviewDialog.h"
#include "logger.h"


CaseReviewDialog::CaseReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseReviewDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    m_player = new VideoPlayer(this);
    m_player->setVideoWidgetContainer(ui->verticalLayout);
    m_player->init();

}

CaseReviewDialog::~CaseReviewDialog()
{
    delete ui;
}
