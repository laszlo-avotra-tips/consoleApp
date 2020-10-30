#ifndef CASEREVIEWDIALOG_H
#define CASEREVIEWDIALOG_H

#include <QDialog>
#include "videoplayer.h"

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
    VideoPlayer* m_player{nullptr};
};

#endif // CASEREVIEWDIALOG_H
