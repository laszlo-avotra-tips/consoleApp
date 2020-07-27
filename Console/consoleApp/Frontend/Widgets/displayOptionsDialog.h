#ifndef DISPLAYOPTIONSDIALOG_H
#define DISPLAYOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class DisplayOptionsDialog;
}

class DisplayOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DisplayOptionsDialog(QWidget *parent = nullptr);
    ~DisplayOptionsDialog();

private slots:
    void on_pushButtonDone_clicked();

    void on_pushButtonBack_clicked();

private:
    Ui::DisplayOptionsDialog *ui;
};

#endif // DISPLAYOPTIONSDIALOG_H
