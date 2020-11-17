#ifndef CASEREVIEWSCREEN_H
#define CASEREVIEWSCREEN_H

#include <QWidget>

#include "videoplayer.h"
#include "Utility/captureListModel.h"
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
    void showEvent(QShowEvent *) override;

signals:
//    void showCapture( const QImage &, const QImage &);
    void currentCaptureChanged( QModelIndex );
    void sendStatusText( QString );
    void sendDeviceName( QString );
//    void displayingCapture();
    void displayOffsetChanged(int dpo);

public slots:
    void captureSelected(QModelIndex ix);
//    void displayCapture(QModelIndex index);
    void updateCaptureCount( void );

private slots:
    void on_pushButtonBack_clicked();
    void setSliderPosition(quint64 position);
    void setSliderRange(quint64 range);
    void updateCaseInfo();
    void updateSliderLabels();

    void on_pushButtonDone_clicked();

    void on_pushButtonRightArrow_clicked();

private:
    void initPlayer();
    void initCapture();
    void showPlayer( bool isVisible);
    void showCapture( bool isVisible);
    void hideUnimplementedButtons();
    void updateCaptureLabel();

private:
    Ui::CaseReviewScreen *ui;
    VideoPlayer* m_player{nullptr};
    quint64 m_duration{0};
    quint64 m_position{0};

private:
    int  m_numCaptures{0};
    bool m_isImageReviewInProgress{false};

    captureItem *m_selectedCaptureItem{nullptr};
    int m_displayOffset{0};

};

#endif // CASEREVIEWSCREEN_H
