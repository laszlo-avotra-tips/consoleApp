#ifndef DISPLAYOPTIONSSCREEN_H
#define DISPLAYOPTIONSSCREEN_H

#include <QWidget>

namespace Ui {
class DisplayOptionsScreen;
}

class DisplayOptionsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayOptionsScreen(QWidget *parent = nullptr);
    ~DisplayOptionsScreen();

private slots:
    void on_pushButtonDone_clicked();

private:
    Ui::DisplayOptionsScreen *ui;
};

#endif // DISPLAYOPTIONSSCREEN_H
