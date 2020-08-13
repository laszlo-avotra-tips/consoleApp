#ifndef DISPLAYOPTIONSDIALOG_H
#define DISPLAYOPTIONSDIALOG_H

#include <QDialog>

class liveScene;
class QGraphicsView;

namespace Ui {
class DisplayOptionsDialog;
}

class DisplayOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DisplayOptionsDialog(QWidget *parent = nullptr);
    ~DisplayOptionsDialog();
    void setScene(liveScene* scene);

signals:
    void setColorModeSepia();
    void setColorModeGray();

private slots:
    void on_pushButtonDone_clicked();

    void on_pushButtonBack_clicked();

    void on_radioButtonDown_clicked(bool checked);

    void on_radioButtonUp_clicked(bool checked);

    void on_radioButtonGrey_clicked(bool checked);

    void on_radioButtonSepia_clicked(bool checked);

private:
    Ui::DisplayOptionsDialog *ui;
    QGraphicsView* m_graphicsView{nullptr};
    liveScene* m_scene{nullptr};
};

#endif // DISPLAYOPTIONSDIALOG_H
