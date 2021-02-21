#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget>
#include <QTimer>

class Backend;
class QGestureEvent;

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

    void on_pushButtonStart_released();

    void on_pushButtonStart_pressed();

    void setPressAndHold();

    bool event(QEvent *event) override;

    bool gestureEvent(QGestureEvent* ge);

private:
    void showEvent(QShowEvent* se) override;
//    void hideEvent(QHideEvent* he) override;

    Ui::StartScreen *ui;
    Backend* m_backend{nullptr};
    bool m_isPressAndHold{false};
    QTimer m_timer;
};

#endif // STARTSCREEN_H
