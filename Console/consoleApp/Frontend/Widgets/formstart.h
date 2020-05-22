#ifndef FORMSTART_H
#define FORMSTART_H

#include <QWidget>

namespace Ui {
class FormStart;
}

class FormStart : public QWidget
{
    Q_OBJECT

public:
    explicit FormStart(QWidget *parent = nullptr);
    ~FormStart();

private slots:
    void on_pushButtonMenu_clicked(bool checked);

    void on_pushButtonPreferences_clicked();

    void on_pushButtonShutdown_clicked();

private:
    Ui::FormStart *ui;
};

#endif // FORMSTART_H
