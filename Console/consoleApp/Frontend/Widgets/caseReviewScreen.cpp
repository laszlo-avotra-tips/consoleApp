#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"

#include "Utility/widgetcontainer.h"
#include "caseInformationModel.h"
#include "logger.h"
#include "sledsupport.h"

#include "captureItemDelegate.h"

#include <QGraphicsPixmapItem>


CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
    initPlayer();
    initCapture();
    showPlayer(false);
    showCapture(false);
    updateCaptureLabel();

    hideUnimplementedButtons();

    ui->captureView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->captureView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->pushButtonRightArrow->hide();
}

void CaseReviewScreen::hideUnimplementedButtons()
{
    ui->pushButtonAnnotate->hide();
    ui->pushButtonMeasure->hide();
    ui->pushButtonSaveImage->hide();
}

void CaseReviewScreen::updateCaptureLabel()
{
    captureListModel &capList = captureListModel::Instance();

    m_numCaptures = capList.rowCount(QModelIndex());
    LOG1(m_numCaptures)

    ui->labelImages->setText( tr( "IMAGES(%1)" ).arg( m_numCaptures ) );

    if(m_numCaptures <= 5){
        ui->pushButtonRightArrow->hide();
        ui->pushButtonLeftArrow->hide();
    } else {
        ui->pushButtonRightArrow->show();
        ui->pushButtonLeftArrow->show();
    }
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
    LOG1("initCapture")
    // set up the list for image captures
    captureListModel &capList = captureListModel::Instance();

    CaptureItemDelegate* crDelegate = new CaptureItemDelegate();
    ui->captureView->setItemDelegate( crDelegate );
    ui->captureView->setModel( &capList );

    connect( ui->captureView, &captureListView::clicked, this, &CaseReviewScreen::captureSelected );

    connect(crDelegate, &CaptureItemDelegate::updateLabel, this, &CaseReviewScreen::updateCaptureLabel);

    // Auto-scroll the list when items are added
    connect( &capList, &captureListModel::rowsInserted, ui->captureView, &captureListView::updateView );

    //scroll
    connect(this, &CaseReviewScreen::displayOffsetChanged, crDelegate, &CaptureItemDelegate::handleDisplayOffset);
}

void CaseReviewScreen::showPlayer(bool isVisible)
{
    if(isVisible){
        ui->framePlayer->show();
        ui->horizontalSlider->show();
    } else {
        ui->framePlayer->hide();
        ui->horizontalSlider->hide();
    }
}

void CaseReviewScreen::showCapture(bool isVisible)
{
    if(isVisible){
        ui->captureScene->show();
    } else {
        ui->captureScene->hide();
    }

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

    QGraphicsScene *scene = new QGraphicsScene();

    ui->captureScene->setScene(scene);
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
//    m_selectedCaptureItem = index.data( Qt::DisplayRole ).value<captureItem *>();

    const int rowNum = index.row() + m_displayOffset;

    LOG1(rowNum)

    captureListModel& capList = captureListModel::Instance();
    capList.setSelectedRow(rowNum);
    update();

    auto itemList = capList.getAllItems();

    m_selectedCaptureItem = itemList.at(rowNum);

    LOG1(rowNum)
    showPlayer(false);
    showCapture(true);

    const auto& imageName{m_selectedCaptureItem->getName()};
    LOG1(imageName)
    QImage image = m_selectedCaptureItem->loadDecoratedImage(imageName).scaledToWidth(1600);

    LOG2(image.size().width(), image.size().height())

    QGraphicsScene *scene = new QGraphicsScene();

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));

    scene->addItem(item);

    ui->captureScene->setScene(scene);
}


void CaseReviewScreen::on_pushButtonDone_clicked()
{
}

void CaseReviewScreen::on_pushButtonRightArrow_clicked()
{
    const int size{5};
    if(m_numCaptures > size + m_displayOffset){
        ++m_displayOffset;
        emit displayOffsetChanged(m_displayOffset);
        update();
    }
}

void CaseReviewScreen::on_pushButtonLeftArrow_clicked()
{
    if(m_displayOffset){
        --m_displayOffset;
        emit displayOffsetChanged(m_displayOffset);
        update();
    }
}
