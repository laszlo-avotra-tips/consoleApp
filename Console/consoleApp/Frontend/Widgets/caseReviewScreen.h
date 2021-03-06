#ifndef CASEREVIEWSCREEN_H
#define CASEREVIEWSCREEN_H

#include <QWidget>

#include "videoplayer.h"
#include "Utility/captureListModel.h"
#include "Utility/clipListModel.h"
#include "capturelistview.h"


namespace Ui {
class CaseReviewScreen;
}

class CaseReviewScreen : public QWidget
{
    Q_OBJECT

public:
    explicit CaseReviewScreen(QWidget *parent = nullptr);
    ~CaseReviewScreen();

signals:
    void sendStatusText( QString );
    void sendDeviceName( QString );
    void displayOffsetChanged(int dpo);

public slots:
    void captureSelected(QModelIndex ix);
    void clipSelected(QModelIndex ix);

private slots:
    void on_pushButtonBack_clicked();
    void setSliderPosition(quint64 position);
    void setSliderRange(quint64 range);
    void updateCaseInfo();
    void updateSliderLabels();

    void on_pushButtonDone_clicked();

    void on_pushButtonRightArrow_clicked();

    void on_pushButtonLeftArrow_clicked();

    void on_pushButtonClipsLeftArrow_clicked();

    void on_pushButtonClipsRightArrow_clicked();

private:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *e) override;

    void initPlayer();
    void initCapture();
    void initClips();
    void showPlayer( bool isVisible);
    void showCapture( bool isVisible);
    void showClip( bool isVisible);
    void hideUnimplementedButtons();
    void updateCaptureLabel();
    void updateClipLabel();

private:
    Ui::CaseReviewScreen *ui;
    VideoPlayer* m_player{nullptr};
    quint64 m_duration{0};
    quint64 m_position{0};

private:
    int  m_numCaptures{0};
    int  m_numClips{0};
    bool m_isImageReviewInProgress{false};

    captureItem *m_selectedCaptureItem{nullptr};
    clipItem *m_selectedClipItem{nullptr};

};

#endif // CASEREVIEWSCREEN_H
