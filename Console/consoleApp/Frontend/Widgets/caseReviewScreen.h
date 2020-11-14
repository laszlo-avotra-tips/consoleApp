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
    void showCapture( const QImage &, const QImage &);
    void currentCaptureChanged( QModelIndex );
    void sendStatusText( QString );
    void sendDeviceName( QString );
    void displayingCapture();

public slots:
    void captureSelected(QModelIndex ix);
    void displayCapture(QModelIndex index);
    void updateCaptureCount( void );

private slots:
    void on_pushButtonBack_clicked();
    void setSliderPosition(quint64 position);
    void setSliderRange(quint64 range);
    void updateCaseInfo();
    void updateSliderLabels();

private:
    void initPlayer();
    void initCapture();
    void showPlayer( bool isVisible);
    void showCapture( bool isVisible);

private:
    Ui::CaseReviewScreen *ui;
    VideoPlayer* m_player{nullptr};
    quint64 m_duration{0};
    quint64 m_position{0};

private:
    int  m_numCaptures{0};
    bool m_isImageReviewInProgress{false};

    captureItem *m_selectedCaptureItem{nullptr};


};

#endif // CASEREVIEWSCREEN_H
