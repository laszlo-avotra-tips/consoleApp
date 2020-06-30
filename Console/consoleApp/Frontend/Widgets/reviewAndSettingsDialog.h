#ifndef REVIEWANDSETTINGSDIALOG_H
#define REVIEWANDSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ReviewAndSettingsDialog;
}

class ReviewAndSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReviewAndSettingsDialog(QWidget *parent = nullptr);
    ~ReviewAndSettingsDialog();

private slots:
    void on_pushButtonExit_clicked();

    void on_pushButtonNext_clicked();

private:
    Ui::ReviewAndSettingsDialog *ui;
};

#endif // REVIEWANDSETTINGSDIALOG_H
