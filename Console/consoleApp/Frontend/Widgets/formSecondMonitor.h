#ifndef FORMSECONDMONITOR_H
#define FORMSECONDMONITOR_H

#include <QWidget>

namespace Ui {
class FormSecondMonitor;
}

class FormSecondMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit FormSecondMonitor(QWidget *parent = nullptr);
    ~FormSecondMonitor();

private:
    Ui::FormSecondMonitor *ui;
};

#endif // FORMSECONDMONITOR_H
