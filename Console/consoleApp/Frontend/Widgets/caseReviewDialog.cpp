#include "caseReviewDialog.h"
#include "ui_caseReviewDialog.h"

#include <QVideoWidget>
#include <QMediaPlayer>
#include <QSlider>
#include <QAbstractButton>
#include <QFileDialog>
#include <QStandardPaths>

CaseReviewDialog::CaseReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseReviewDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    m_videoWidget = ui->videoWidget; //new QVideoWidget(this);
    m_playButton = ui->pushButtonPlay;

}

CaseReviewDialog::~CaseReviewDialog()
{
    delete ui;
}

void CaseReviewDialog::on_pushButtonOpen_clicked()
{
    openFile();
}

void CaseReviewDialog::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    QStringList supportedMimeTypes = m_mediaPlayer->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty())
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        setUrl(fileDialog.selectedUrls().constFirst());
}

void CaseReviewDialog::setUrl(const QUrl &url)
{
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    m_mediaPlayer->setMedia(url);
    m_playButton->setEnabled(true);
}

void CaseReviewDialog::play()
{
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    default:
        m_mediaPlayer->play();
        break;
    }
}

