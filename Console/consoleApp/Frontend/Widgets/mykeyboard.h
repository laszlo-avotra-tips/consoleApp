#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QDialog>

namespace Ui {
class MyKeyboard;
}

class MyKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit MyKeyboard(QWidget *parent = nullptr);
    ~MyKeyboard();
    QString name();

private slots:
    void on_pushButtona_clicked();

private:
    Ui::MyKeyboard *ui;
};

#endif // KEYBOARD_H
