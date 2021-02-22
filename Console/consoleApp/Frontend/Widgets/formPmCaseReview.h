#ifndef FORMPMCASEREVIEW_H
#define FORMPMCASEREVIEW_H

#include <QWidget>

class QGraphicsScene;

namespace Ui {
class FormPmCaseReview;
}

class FormPmCaseReview : public QWidget
{
    Q_OBJECT

public:
    explicit FormPmCaseReview(QWidget *parent = nullptr);
    ~FormPmCaseReview();
    void setScene(QGraphicsScene* scene);
    void showCapture(bool isVisible);

private:
    Ui::FormPmCaseReview *ui;
};

#endif // FORMPMCASEREVIEW_H
