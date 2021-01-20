#ifndef FORMSECONDMONITOR_H
#define FORMSECONDMONITOR_H

#include <QWidget>

class QGraphicsView;
class liveScene;

namespace Ui {
class FormSecondMonitor;
}

class FormSecondMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit FormSecondMonitor(QWidget *parent = nullptr);
    ~FormSecondMonitor();
    void setScene(liveScene* scene);

private:
    Ui::FormSecondMonitor *ui;
};

#endif // FORMSECONDMONITOR_H
