#ifndef FORMNAVIGATOR_H
#define FORMNAVIGATOR_H

#include <QWidget>

namespace Ui {
class FormNavigator;
}

class FormNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit FormNavigator(QWidget *parent = nullptr);
    ~FormNavigator();
    void display();

private:
    Ui::FormNavigator *ui;
};

#endif // FORMNAVIGATOR_H
