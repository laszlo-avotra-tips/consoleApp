#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"

#include "Utility/widgetcontainer.h"
#include "caseInformationModel.h"
#include "logger.h"
#include "sledsupport.h"
#include "captureItemDelegate.h"
#include "Utility/captureListModel.h"
#include "clipItemDelegate.h"
#include "Utility/clipListModel.h"

#include <QGraphicsPixmapItem>


CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
    initPlayer();
    initCapture();
    initClips();

    showPlayer(false);
    showCapture(false);
    showClip(false);

    updateCaptureLabel();
    updateClipLabel();

    hideUnimplementedButtons();

    ui->captureView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->captureView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->clipsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->clipsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

    m_numCaptures = capList.countOfCapuredItems(); //capList.rowCount(QModelIndex());
//    LOG1(m_numCaptures)

    ui->labelImages->setText( tr( "IMAGES(%1)" ).arg( m_numCaptures ) );

    if( (m_numCaptures - capList.getRowOffset()) <= 5 ){
        ui->pushButtonRightArrow->hide();
    } else {
        ui->pushButtonRightArrow->show();
    }

    if(capList.getRowOffset() > 0){
        ui->pushButtonLeftArrow->show();
    } else {
        ui->pushButtonLeftArrow->hide();
    }
}

void CaseReviewScreen::updateClipLabel()
{
    clipListModel& clipList = clipListModel::Instance();

    m_numClips = clipList.countOfClipItems();

//    LOG1(m_numClips)

    ui->labelLoops->setText( tr( "LOOPS(%1)" ).arg( m_numClips ) );

    if( (m_numClips - clipList.getRowOffset()) <= 5 ){
        ui->pushButtonClipsRightArrow->hide();
    } else {
        ui->pushButtonClipsRightArrow->show();
    }

    if(clipList.getRowOffset() > 0){
        ui->pushButtonClipsLeftArrow->show();
    } else {
        ui->pushButtonClipsLeftArrow->hide();
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

void CaseReviewScreen::initClips()
{
    LOG1("initClips")
    clipListModel& clipList = clipListModel::Instance();

    ClipItemDelegate* clipItemDelegate = new ClipItemDelegate();
    ui->clipsView->setItemDelegate(clipItemDelegate);
    ui->clipsView->setModel(&clipList);

    connect( ui->clipsView, &clipListView::clicked, this, &CaseReviewScreen::clipSelected );

    connect(clipItemDelegate, &ClipItemDelegate::updateLabel, this, &CaseReviewScreen::updateClipLabel);

    // Auto-scroll the list when items are added
    connect( &clipList, &clipListModel::rowsInserted, ui->clipsView, &clipListView::updateView );

    //scroll
    connect(this, &CaseReviewScreen::displayOffsetChanged, clipItemDelegate, &ClipItemDelegate::handleDisplayOffset);
}

void CaseReviewScreen::showPlayer(bool isVisible)
{
    LOG1(isVisible)
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

void CaseReviewScreen::showClip(bool isVisible)
{
    showPlayer(isVisible);
}


CaseReviewScreen::~CaseReviewScreen()
{
    delete m_player;
    delete ui;
}

void CaseReviewScreen::showEvent(QShowEvent * e)
{
    updateCaseInfo();
    updateCaptureLabel();
    QWidget::showEvent(e);

    auto& model = captureListModel::Instance();

//    if(model.getSelectedRow() <= 0)
    {

        QGraphicsScene *scene = new QGraphicsScene();

        ui->captureScene->setScene(scene);
    }
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

    captureListModel& capList = captureListModel::Instance();
    const int rowNum = index.row() + capList.getRowOffset();

    LOG2(rowNum,m_numCaptures)

    capList.setSelectedRow(rowNum);
    update();

    auto itemList = capList.getAllItems();

    m_selectedCaptureItem = itemList.at(rowNum);

    LOG2(rowNum, m_selectedCaptureItem)
    showClip(false);
    showCapture(true);

    if(m_selectedCaptureItem){

        const auto& imageName{m_selectedCaptureItem->getName()};
        LOG1(imageName)
        QImage image = m_selectedCaptureItem->loadDecoratedImage(imageName).scaledToWidth(1600);

        LOG2(image.size().width(), image.size().height())

        QGraphicsScene *scene = new QGraphicsScene();

        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));

        scene->addItem(item);

        ui->captureScene->setScene(scene);
    }
}

void CaseReviewScreen::clipSelected(QModelIndex index)
{
    clipListModel& clipList = clipListModel::Instance();
    const int rowNum = index.row() + clipList.getRowOffset();

    LOG2(rowNum,m_numClips)

    clipList.setSelectedRow(rowNum);
    update();

    auto itemList = clipList.getAllItems();

    m_selectedClipItem = itemList.at(rowNum);

    LOG2(rowNum, m_selectedClipItem)
    showClip(true);
    showCapture(false);

    if(m_selectedClipItem){

        const auto& loopName{m_selectedClipItem->getName()};
        LOG1(loopName)
//        QImage image = m_selectedCaptureItem->loadDecoratedImage(loopName).scaledToWidth(1600);

//        LOG2(image.size().width(), image.size().height())

//        QGraphicsScene *scene = new QGraphicsScene();

//        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));

//        scene->addItem(item);

//        ui->captureScene->setScene(scene);
    }
}


void CaseReviewScreen::on_pushButtonDone_clicked()
{
    WidgetContainer::instance()->gotoScreen("mainScreen");
}

void CaseReviewScreen::on_pushButtonRightArrow_clicked()
{
    const int size{5};
    captureListModel& capList = captureListModel::Instance();
    if(m_numCaptures > size + capList.getRowOffset()){
        update();
        capList.setRowOffset(capList.getRowOffset() + 1);
    }
}

void CaseReviewScreen::on_pushButtonLeftArrow_clicked()
{
    captureListModel& capList = captureListModel::Instance();
    if(capList.getRowOffset()){
        update();
        capList.setRowOffset(capList.getRowOffset() - 1);
    }
}

void CaseReviewScreen::on_pushButtonClipsRightArrow_clicked()
{
    const int size{5};
    clipListModel& clipList = clipListModel::Instance();
    if(m_numClips > size + clipList.getRowOffset()){
        update();
        clipList.setRowOffset(clipList.getRowOffset() + 1);
    }
}

void CaseReviewScreen::on_pushButtonClipsLeftArrow_clicked()
{
    clipListModel& clipList = clipListModel::Instance();
    if(clipList.getRowOffset()){
        update();
        clipList.setRowOffset(clipList.getRowOffset() - 1);
    }
}
