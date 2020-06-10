#ifndef SCREENNAVIGATOR_H
#define SCREENNAVIGATOR_H

#include <QWidget>

namespace Ui {
class ScreenNavigator;
}

class ScreenNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenNavigator(QWidget *parent = nullptr);
    ~ScreenNavigator();
    void display();

private:
    void setStylesheet();

private:
    Ui::ScreenNavigator *ui;
};

#endif // FORMNAVIGATOR_H
