#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <vector>
#include <map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonFlip_clicked();

    void on_pushButtonMenu_clicked();

    void on_pushButtonPage1_clicked();

    void on_pushButtonPage2_clicked();

    void on_pushButtonExitL_clicked();

    int getSceneSize();

private:
    void flipColumns();

private:
    Ui::MainWindow *ui;
    const std::pair<int,int> m_widthHeight{3,2};
    const int m_sceneSize{2160};

};
#endif // MAINWINDOW_H
