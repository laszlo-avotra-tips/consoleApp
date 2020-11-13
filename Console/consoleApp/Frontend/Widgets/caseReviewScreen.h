#ifndef CASEREVIEWSCREEN_H
#define CASEREVIEWSCREEN_H

#include <QWidget>

#include "videoplayer.h"
#include "Utility/captureListModel.h"

class CaptureItem;

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

private slots:
    void on_pushButtonBack_clicked();
    void setSliderPosition(quint64 position);
    void setSliderRange(quint64 range);
    void updateCaseInfo();
    void updateSliderLabels();

private:
    void initPlayer();
    void initCapture();

private:
    Ui::CaseReviewScreen *ui;
    VideoPlayer* m_player{nullptr};
    quint64 m_duration{0};
    quint64 m_position{0};

private:
    int  m_numCaptures{0};
    bool m_isImageReviewInProgress{false};

    CaptureItem *m_selectedCaptureItem{nullptr};


};

#endif // CASEREVIEWSCREEN_H
