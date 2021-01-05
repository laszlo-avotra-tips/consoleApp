#ifndef FORMSECONDMONITOR_H
#define FORMSECONDMONITOR_H

#include <QWidget>

class QGraphicsView;

namespace Ui {
class FormSecondMonitor;
}

class FormSecondMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit FormSecondMonitor(QWidget *parent = nullptr);
    ~FormSecondMonitor();
    QGraphicsView* getGraphicsView();

private:
    Ui::FormSecondMonitor *ui;
};

#endif // FORMSECONDMONITOR_H
