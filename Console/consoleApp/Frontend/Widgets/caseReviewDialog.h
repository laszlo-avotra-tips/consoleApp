#ifndef CASEREVIEWDIALOG_H
#define CASEREVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class CaseReviewDialog;
}

class CaseReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaseReviewDialog(QWidget *parent = nullptr);
    ~CaseReviewDialog();

private slots:
    void on_pushButtonOpen_clicked();

private:
    Ui::CaseReviewDialog *ui;
};

#endif // CASEREVIEWDIALOG_H
