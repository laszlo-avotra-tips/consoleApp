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
    initCapture();
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

//    connect( ui->captureView, SIGNAL( clicked( QModelIndex ) ), this, SLOT( captureSelected(QModelIndex) ) );
//    connect( ui->captureView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );
    connect( ui->captureView, &captureListView::clicked, this, &CaseReviewScreen::captureSelected );
    connect( ui->captureView, &captureListView::doubleClicked, this, &CaseReviewScreen::displayCapture );

    // Auto-scroll the list when items are added
//    connect( &capList, SIGNAL( rowsInserted( QModelIndex, int, int ) ), ui->captureView, SLOT( updateView( QModelIndex, int, int ) ) );
    connect( &capList, &captureListModel::rowsInserted, ui->captureView, &captureListView::updateView );

    // keyboard keys change the selection
//    connect( ui->captureView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
//             this, SLOT(captureSelected(const QModelIndex &)) );
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

/*
 * captureSelected()
 *
 * One of the items in the preview list has been selected.
 * Highlight it.
 */
void CaseReviewScreen::captureSelected( QModelIndex index )
{
    m_selectedCaptureItem = index.data( Qt::DisplayRole ).value<captureItem *>();

//    ui->selectedCaptureLineEdit->setText( selectedCaptureItem->getTag() );
    emit currentCaptureChanged( index );
    LOG1(index.row())
}

/*
 * displayCapture()
 *
 * Load and display the selected image on the Technician and Physican monitors.
 */
void CaseReviewScreen::displayCapture( QModelIndex index )
{
    captureItem *item = index.data( Qt::DisplayRole ).value<captureItem *>();
    LOG1(index.row())

    if( item )
    {
        emit showCapture( item->loadSector( item->getName() ), item->loadWaterfall( item->getName() ) );

        QString str = QString( tr( "REVIEW: %1 (%2)" ) ).arg( item->getTag() ).arg( item->getIdNumber(), 3, 10, QLatin1Char( '0' ) );
        emit sendStatusText( str );

        // update the label to the review device
        emit sendDeviceName( item->getDeviceName() );
        emit displayingCapture();
//        emit sendReviewImageCalibrationFactors( selectedCaptureItem->getPixelsPerMm(), zoomFactor );

        if( !m_isImageReviewInProgress )
        {
            LOG( INFO, "Image Review started" );
//            disconnect( ui->captureView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( displayCapture(QModelIndex) ) );
            disconnect( ui->captureView, &captureListView::doubleClicked, this, &CaseReviewScreen::displayCapture );

//            connect(    ui->captureView, SIGNAL( clicked( QModelIndex ) ),       this, SLOT( displayCapture(QModelIndex) ) );
            connect( ui->captureView, &captureListView::doubleClicked, this, &CaseReviewScreen::displayCapture );

            // keyboard keys change the selection
//            connect( ui->captureView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
//                     this, SLOT(displayCapture(const QModelIndex &)) );

            m_isImageReviewInProgress = true;
        }
    }
}

/*
 * updateCaptureCount
 */
void CaseReviewScreen::updateCaptureCount( void )
{
    m_numCaptures++;
    ui->labelImages->setText( tr( "Images (%1)" ).arg( m_numCaptures ) );
}

