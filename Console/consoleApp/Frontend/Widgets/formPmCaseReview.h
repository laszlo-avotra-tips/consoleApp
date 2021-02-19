#ifndef FORMPMCASEREVIEW_H
#define FORMPMCASEREVIEW_H

#include <QWidget>

namespace Ui {
class FormPmCaseReview;
}

class FormPmCaseReview : public QWidget
{
    Q_OBJECT

public:
    explicit FormPmCaseReview(QWidget *parent = nullptr);
    ~FormPmCaseReview();

private:
    Ui::FormPmCaseReview *ui;
};

#endif // FORMPMCASEREVIEW_H
