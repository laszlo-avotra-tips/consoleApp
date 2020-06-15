#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget>

class Backend;
class frontend;

namespace Ui {
class StartScreen;
}

class StartScreen : public QWidget
{
    Q_OBJECT

public:
    explicit StartScreen(QWidget *parent = nullptr);
    ~StartScreen();

private slots:
    void on_pushButtonMenu_clicked(bool checked);

    void on_pushButtonPreferences_clicked();

    void on_pushButtonShutdown_clicked();

    void on_pushButtonStart_clicked();

private:
    void showEvent(QShowEvent* se) override;
    void hideEvent(QHideEvent* he) override;

    void startDaq(frontend *fe);

    Ui::StartScreen *ui;
    Backend* m_backend{nullptr};
};

#endif // STARTSCREEN_H
