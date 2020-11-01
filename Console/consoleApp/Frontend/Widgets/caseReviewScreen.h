#ifndef CASEREVIEWSCREEN_H
#define CASEREVIEWSCREEN_H

#include <QWidget>

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
};

#endif // CASEREVIEWSCREEN_H
