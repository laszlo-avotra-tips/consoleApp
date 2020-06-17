#ifndef OPACSCREEN_H
#define OPACSCREEN_H

#include <QWidget>

namespace Ui {
class OpacScreen;
}

class OpacScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OpacScreen(QWidget *parent = nullptr);
    ~OpacScreen();

private:
    void init();
    Ui::OpacScreen *ui;
};

#endif // OPACSCREEN_H
