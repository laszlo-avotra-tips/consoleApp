#ifndef OPACSCREEN_H
#define OPACSCREEN_H

#include <QWidget>

namespace Ui {
class OpacScreen;
}

class OpaqueScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OpaqueScreen(QWidget *parent = nullptr);
    ~OpaqueScreen();

private:
    void init();
    Ui::OpacScreen *ui;
};

#endif // OPACSCREEN_H
