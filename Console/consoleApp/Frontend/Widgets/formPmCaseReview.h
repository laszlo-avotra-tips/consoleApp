#ifndef FORMPMCASEREVIEW_H
#define FORMPMCASEREVIEW_H

#include <QWidget>

class QGraphicsScene;
class QVBoxLayout;

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
    QVBoxLayout *getVideoWidgetContainer();
    void updateSliderLabels(const QString& time, const QString& duration);

private:
    Ui::FormPmCaseReview *ui;
};

#endif // FORMPMCASEREVIEW_H
