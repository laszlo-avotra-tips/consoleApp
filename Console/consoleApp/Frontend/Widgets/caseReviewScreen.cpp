#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"
#include "Utility/widgetcontainer.h"
#include "caseInformationModel.h"
#include "caseReviewModel.h"
#include "logger.h"

#include <QMediaPlaylist>

CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
    m_player = new VideoPlayer(this);
    m_player->setVideoWidgetContainer(ui->verticalLayout);
    m_player->init();

    auto* slider = ui->horizontalSlider;
    connect(slider, &QAbstractSlider::sliderMoved, m_player, &VideoPlayer::setPosition);
    connect(m_player, &VideoPlayer::updatePosition, this, &CaseReviewScreen::setSliderPosition);
    connect(m_player, &VideoPlayer::updateDuration, this, &CaseReviewScreen::setSliderRange);

    connect(ui->pushButtonPlay, &QAbstractButton::clicked, m_player, &VideoPlayer::play);

}

CaseReviewScreen::~CaseReviewScreen()
{
    delete m_player;
    delete ui;
}

void CaseReviewScreen::showEvent(QShowEvent * e)
{
    updateCaseInfo();
    QWidget::showEvent(e);
}

void CaseReviewScreen::on_pushButtonBack_clicked()
{
    WidgetContainer::instance()->gotoScreen("mainScreen");
}

void CaseReviewScreen::setSliderPosition(quint64 position)
{
    const auto& playlist = m_player->getPlaylist(); //CaseReviewModel::instance()->getPlaylist();
    int index = playlist.currentIndex();
    m_position = position + index * 30000;
    ui->horizontalSlider->setValue(m_position);
    updateSliderLabels();
    LOG2(index,m_position)
}

void CaseReviewScreen::setSliderRange(quint64 range)
{
    const auto& playlist = m_player->getPlaylist(); //CaseReviewModel::instance()->getPlaylist();
    int count = playlist.mediaCount();
    m_duration = range * count;
    ui->horizontalSlider->setRange(0, m_duration);
    updateSliderLabels();

    LOG2(count, m_duration);
}

void CaseReviewScreen::updateCaseInfo()
{
    const auto& physicianName = CaseInformationModel::instance()->selectedPhysicianName();
    LOG1(physicianName)
    ui->labelPhysicianNameValue->setText(physicianName);

    const auto& caseDateAndTime = CaseInformationModel::instance()->dateAndTime();
    LOG1(caseDateAndTime)
    ui->labelDateAndTimeValue->setText(caseDateAndTime);

    const auto& patientId = CaseInformationModel::instance()->patientId();
    LOG1(patientId)
    ui->labelPatientIdValue->setText(patientId);

}

void CaseReviewScreen::updateSliderLabels()
{
//    const auto& playlist = m_player->getPlaylist(); //CaseReviewModel::instance()->getPlaylist();
//    int count = playlist.mediaCount();
//    int index = playlist.currentIndex();

    int durationInSeconds = m_duration / 1000;
    short durationMinutes = durationInSeconds / 60;
    short durationSeconds = durationInSeconds % 60;

    const QString& duration = QString("%1:%2").arg(durationMinutes,2,10,QLatin1Char('0')).arg(durationSeconds,2,10,QLatin1Char('0'));
    ui->labelDuration->setText(duration);

    int timeInSeconds = m_position / 1000;
    short timeMinutes = timeInSeconds / 60;
    short timeSeconds = timeInSeconds % 60;

    const QString& time = QString("%1:%2").arg(timeMinutes,2,10,QLatin1Char('0')).arg(timeSeconds,2,10,QLatin1Char('0'));
    ui->labelTime->setText(time);
}
