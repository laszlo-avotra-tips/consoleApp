#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"

#include "Utility/widgetcontainer.h"
#include "caseInformationModel.h"
#include "logger.h"

#include "captureItemDelegate.h"

CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
    initPlayer();
}


/* init player */
void CaseReviewScreen::initPlayer()
{

    m_player = new VideoPlayer(this);
    m_player->setVideoWidgetContainer(ui->verticalLayout);
    m_player->init();

    auto* slider = ui->horizontalSlider;
    connect(slider, &QAbstractSlider::sliderMoved, m_player, &VideoPlayer::setPosition);
    connect(m_player, &VideoPlayer::updatePosition, this, &CaseReviewScreen::setSliderPosition);
    connect(m_player, &VideoPlayer::updateDuration, this, &CaseReviewScreen::setSliderRange);

    connect(ui->pushButtonPlay, &QAbstractButton::clicked, m_player, &VideoPlayer::play);

}

/*
 * Image Captures
 */
void CaseReviewScreen::initCapture()
{
    // set up the list for image captures
    captureListModel &capList = captureListModel::Instance();

    ui->captureView->setItemDelegate( new CaptureItemDelegate() );
    ui->captureView->setModel( &capList );

    connect( ui->captureView, SIGNAL( clicked( QModelIndex ) ), this, SLOT( captureSelected(QModelIndex) ) );
    connect( ui->captureView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );

    // Auto-scroll the list when items are added
    connect( &capList, SIGNAL( rowsInserted( QModelIndex, int, int ) ), ui->captureView, SLOT( updateView( QModelIndex, int, int ) ) );

    // keyboard keys change the selection
    connect( ui->captureView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
             this, SLOT(captureSelected(const QModelIndex &)) );
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
    m_position = position;
     ui->horizontalSlider->setValue(position);
     updateSliderLabels();
}

void CaseReviewScreen::setSliderRange(quint64 range)
{
    m_duration = range;
    ui->horizontalSlider->setRange(0, range);
    updateSliderLabels();
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
