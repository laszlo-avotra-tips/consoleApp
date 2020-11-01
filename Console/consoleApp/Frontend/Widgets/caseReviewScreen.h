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

private:
    Ui::CaseReviewScreen *ui;
    VideoPlayer* m_player{nullptr};
};

#endif // CASEREVIEWSCREEN_H
