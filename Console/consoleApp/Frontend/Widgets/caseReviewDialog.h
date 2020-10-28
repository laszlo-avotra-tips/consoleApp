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

private:
    Ui::CaseReviewDialog *ui;
};

#endif // CASEREVIEWDIALOG_H
