#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "signalmanager.h"

#include <QWidget>
#include <vector>
#include <map>

class frontend;
class liveScene;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isVisible() const;
    void setScene(liveScene* scene);


    void setDeviceLabel();

private slots:
    void on_pushButtonFlip_clicked();

    void on_pushButtonMenu_clicked();

    int getSceneWidth();

    QSize getSceneSize();

    void on_pushButtonEndCase_clicked();

    void on_pushButtonDownArrow_clicked();
    void on_pushButtonCondensUp_clicked();

    void on_pushButtonCapture_clicked();

    void on_pushButtonSettings_clicked();

private:
    void flipColumns();
    void toggleNavigationButtons(const std::vector<QWidget*>& buttons);
//    void startDaq();

private:
    Ui::MainWindow *ui;
    QGraphicsView* m_graphicsView{nullptr};
    liveScene* m_scene{nullptr};

    const std::pair<int,int> m_widthHeight{3,2};
    const int m_sceneWidth{2160};
    const QSize m_sceneSize{2160,2160};

    std::vector<QWidget*> m_navigationButtons;

    frontend* m_frontEndWindow;

};
#endif // MAINWINDOW_H
