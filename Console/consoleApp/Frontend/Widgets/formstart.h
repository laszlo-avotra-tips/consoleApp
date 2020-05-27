#ifndef FORMSTART_H
#define FORMSTART_H

#include <QWidget>
#include <backend.h>

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

    void on_pushButtonStart_clicked();

private:
    Ui::FormStart *ui;
    Backend m_backend;
};

#endif // FORMSTART_H
