#include "caseReviewScreen.h"
#include "ui_caseReviewScreen.h"

#include "Utility/widgetcontainer.h"
#include "caseInformationModel.h"
#include "logger.h"
#include "captureItemDelegate.h"
#include "Utility/captureListModel.h"
#include "clipItemDelegate.h"
#include "Utility/clipListModel.h"
#include "Utility/octFrameRecorder.h"
#include "displayManager.h"

#include <QGraphicsPixmapItem>
#include <QShowEvent>
#include <QHideEvent>


CaseReviewScreen::CaseReviewScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaseReviewScreen)
{
    ui->setupUi(this);
    initPlayer();
    initCapture();
    initClips();

    showPlayer(false);
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
    const bool recorderIsOn{OctFrameRecorder::instance()->recorderIsOn()};
    QString labelLoopsText = QString( tr("LOOPS(%1)").arg(m_numClips));

    if(recorderIsOn){
        labelLoopsText = labelLoopsText + QString( tr(" - Recording in progress"));
    }
    ui->labelLoops->setText( labelLoopsText );

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
    m_player->setPmVideoWidgetContainer(DisplayManager::instance()->getVideoWidgetContainer());
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
    connect(this, &CaseReviewScreen::displayOffsetChanged, clipItemDelegate, &ClipItemDelegate::handleDisplayOffset);//    showClip(false);
}

void CaseReviewScreen::showPlayer(bool isVisible)
{
    LOG1(isVisible)
    const bool recorderIsOn{OctFrameRecorder::instance()->recorderIsOn()};
    ui->clipsView->setEnabled(!recorderIsOn);
    if(recorderIsOn){
        ui->labelLoops->setStyleSheet("color:gray");
    }else{
        ui->labelLoops->setStyleSheet("color:white");
    }
    if(isVisible){
        ui->framePlayer->show();
        ui->horizontalSlider->show();
        clipListModel& clipList = clipListModel::Instance();
        QString fn = clipList.getThumbnailDir();
        if(m_selectedClipItem){
            const auto& loopName{m_selectedClipItem->getName()};
            const QString videoFileName = QString("file:///%1/%2/%3.mp4").arg(fn).arg(loopName).arg(loopName);
            if(m_selectedClipItem->getIsReady()){
                const QUrl url(videoFileName);
                m_player->setUrl(url);
                LOG1(url.toString());
                ui->pushButtonPlay->setChecked(true);
                ui->pushButtonPlay->show();
                ui->pushButtonPlay->clicked();
            } else {
                ui->pushButtonPlay->hide();
            }
        }

    } else {
        ui->framePlayer->hide();
        ui->horizontalSlider->hide();
    }
}

void CaseReviewScreen::showCapture(bool isVisible)
{
    captureListModel &capList = captureListModel::Instance();
    capList.setIsSelected(isVisible);
    if(isVisible){
        ui->captureScene->show();
        QString yellowBorder("border:2px solid rgb(245,196,0);");
        ui->captureScene->setStyleSheet(yellowBorder);
    } else {
        ui->captureScene->hide();
    }
    DisplayManager::instance()->showCapture(isVisible);
}

void CaseReviewScreen::showClip(bool isVisible)
{
    clipListModel& clipList = clipListModel::Instance();
    clipList.setIsSelected(isVisible);
    showPlayer(isVisible);
}


CaseReviewScreen::~CaseReviewScreen()
{
    delete m_player;
    delete ui;
}

void CaseReviewScreen::showEvent(QShowEvent * e)
{

    QWidget::showEvent(e);
    if(e->type() == QEvent::Show)
    {
        updateCaseInfo();
        updateCaptureLabel();

        showClip(false);
        showPlayer(false);

        QGraphicsScene *scene = new QGraphicsScene();

        ui->captureScene->setScene(scene);
        DisplayManager::instance()->setScene(scene);

        DisplayManager::instance()->showOnTheSecondMonitor("caseReview");
    }

}

void CaseReviewScreen::hideEvent(QHideEvent * e)
{
    QWidget::hideEvent(e);
    if(e->type() == QEvent::Hide)
    {
        DisplayManager::instance()->showOnTheSecondMonitor("liveData");
    }
}

void CaseReviewScreen::on_pushButtonBack_clicked()
{
    LOGUA;
    WidgetContainer::instance()->gotoScreen("mainScreen");
}

void CaseReviewScreen::setSliderPosition(quint64 position)
{
     m_position = position;
     ui->horizontalSlider->setValue(position);
     updateSliderLabels();
     DisplayManager::instance()->setSliderPosition(position);
}

void CaseReviewScreen::setSliderRange(quint64 range)
{
    m_duration = range;
    ui->horizontalSlider->setRange(0, range);
    updateSliderLabels();
    DisplayManager::instance()->setSliderRange(range);
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

    DisplayManager::instance()->updateSliderLabels(time,duration);

    ui->pushButtonPlay->setChecked(m_player->isPlaying());
}

/*
 * captureSelected()
 *
 * One of the items in the preview list has been selected.
 * Highlight it.
 */
void CaseReviewScreen::captureSelected( QModelIndex index )
{
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
        QImage imagePm = image.scaledToWidth(1000);

        LOG2(image.size().width(), image.size().height())

        QGraphicsScene *scene = new QGraphicsScene(this);
        QGraphicsScene *scenePm = new QGraphicsScene(this);

        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        QGraphicsPixmapItem* itemPm = new QGraphicsPixmapItem(QPixmap::fromImage(imagePm));

        scene->addItem(item);
        scenePm->addItem(itemPm);

        ui->captureScene->setScene(scene);
        DisplayManager::instance()->setScene(scenePm);
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
    }
}


void CaseReviewScreen::on_pushButtonDone_clicked()
{
    LOGUA;
    WidgetContainer::instance()->gotoScreen("mainScreen");
}

void CaseReviewScreen::on_pushButtonRightArrow_clicked()
{
    LOGUA;
    const int size{5};
    captureListModel& capList = captureListModel::Instance();
    if(m_numCaptures > size + capList.getRowOffset()){
        update();
        capList.setRowOffset(capList.getRowOffset() + 1);
    }
}

void CaseReviewScreen::on_pushButtonLeftArrow_clicked()
{
    LOGUA;
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
    LOGUA;
    clipListModel& clipList = clipListModel::Instance();
    if(clipList.getRowOffset()){
        update();
        clipList.setRowOffset(clipList.getRowOffset() - 1);
    }
}
