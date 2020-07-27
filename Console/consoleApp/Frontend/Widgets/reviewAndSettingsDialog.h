#ifndef REVIEWANDSETTINGSDIALOG_H
#define REVIEWANDSETTINGSDIALOG_H

#include <QDialog>
#include <vector>

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

    void on_pushButtonDisplayOptions_clicked(bool checked);

    void on_pushButtonCaseReview_clicked(bool checked);

    void on_pushButtonDeviceSelect_clicked(bool checked);

    void on_pushButtonCaseInformation_clicked(bool checked);

private:
    void showLastButtonSelected(QPushButton* button, bool isChecked);

    Ui::ReviewAndSettingsDialog *ui;
    bool m_isSelection{false};
    std::vector<QPushButton*> m_selectionButtons;
    QString m_selection;
};

#endif // REVIEWANDSETTINGSDIALOG_H
