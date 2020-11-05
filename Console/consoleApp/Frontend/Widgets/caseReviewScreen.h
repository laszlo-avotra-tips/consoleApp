#ifndef CASEREVIEWSCREEN_H
#define CASEREVIEWSCREEN_H

#include <QWidget>

#include "videoplayer.h"


namespace Ui {
class CaseReviewScreen;
}

class CaseReviewScreen : public QWidget
{
    Q_OBJECT

public:
    explicit CaseReviewScreen(QWidget *parent = nullptr);
    ~CaseReviewScreen();

private slots:
    void on_pushButtonBack_clicked();
    void setSliderPosition(quint64 position);
    void setSliderRange(quint64 range);
    void updateCaseInfo();
    void displayTimeLeft();

private:
    Ui::CaseReviewScreen *ui;
    VideoPlayer* m_player{nullptr};
    quint64 m_duration{0};
    quint64 m_position{0};

};

#endif // CASEREVIEWSCREEN_H
