#include "caseReviewDialog.h"
#include "ui_caseReviewDialog.h"
#include "logger.h"

#include <QVideoWidget>
#include <QMediaPlayer>
#include <QSlider>
#include <QAbstractButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QStyle>


CaseReviewDialog::CaseReviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaseReviewDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

    m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    QVideoWidget *videoWidget = new QVideoWidget;

    QAbstractButton *openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &CaseReviewDialog::openFile);

    m_playButton = new QPushButton;
    m_playButton->setEnabled(false);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(m_playButton, &QAbstractButton::clicked,
            this, &CaseReviewDialog::play);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setRange(0, 0);

    connect(m_positionSlider, &QAbstractSlider::sliderMoved,
            this, &CaseReviewDialog::setPosition);

    QBoxLayout *controlLayout = ui->verticalLayout; //new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_positionSlider);

    QBoxLayout *layout = ui->verticalLayout;//new QVBoxLayout;
    layout->addWidget(videoWidget);
    layout->addLayout(controlLayout);

//    setLayout(layout);

    m_mediaPlayer->setVideoOutput(videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged,
            this, &CaseReviewDialog::mediaStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &CaseReviewDialog::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &CaseReviewDialog::durationChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &CaseReviewDialog::handleError);

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

void CaseReviewDialog::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void CaseReviewDialog::positionChanged(qint64 position)
{
    m_positionSlider->setValue(position);
}

void CaseReviewDialog::durationChanged(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
}

void CaseReviewDialog::setPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void CaseReviewDialog::handleError()
{
    m_playButton->setEnabled(false);
    const QString errorString = m_mediaPlayer->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(m_mediaPlayer->error()));
    else
        message += errorString;
//    m_errorLabel->setText(message);
    LOG1(message)
}

