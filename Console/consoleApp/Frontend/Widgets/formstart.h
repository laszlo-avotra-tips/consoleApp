#ifndef FORMSTART_H
#define FORMSTART_H

#include <QWidget>

class Backend;

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
    int showCaseInfoDialog();
    int showDeviceWizard();

    Ui::FormStart *ui;
    Backend* m_backend{nullptr};
};

#endif // FORMSTART_H
