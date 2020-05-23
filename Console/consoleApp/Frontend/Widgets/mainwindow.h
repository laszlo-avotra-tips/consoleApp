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

    int getSceneWidth();

    QSize getSceneSize();

    void on_pushButtonEndCase_clicked();


    void on_pushButtonDownArrow_clicked();

private:
    void flipColumns();
    void toggleNavigationButtons(const std::vector<QWidget*>& buttons);

private:
    Ui::MainWindow *ui;
    const std::pair<int,int> m_widthHeight{3,2};
    const int m_sceneWidth{2110};
    const QSize m_sceneSize{2110,2110};

    std::vector<QWidget*> m_navigationButtons;

};
#endif // MAINWINDOW_H
