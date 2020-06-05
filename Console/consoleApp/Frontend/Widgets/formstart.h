#ifndef FORMSTART_H
#define FORMSTART_H

#include <QWidget>

class Backend;
class frontend;

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
    void showEvent(QShowEvent* se) override;
    void hideEvent(QHideEvent* he) override;

    int showCaseInfoDialog();
    int showDeviceWizard();
    void startDaq(frontend *fe);

    Ui::FormStart *ui;
    Backend* m_backend{nullptr};
};

#endif // FORMSTART_H
