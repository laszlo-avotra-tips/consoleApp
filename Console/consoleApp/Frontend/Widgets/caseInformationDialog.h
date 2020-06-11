#ifndef CASEINFORMATIONDIALOG_H
#define CASEINFORMATIONDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>

class SelectDialog;

namespace Ui {
class CaseInformationDialog;
}

class CaseInformationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaseInformationDialog(QWidget *parent = nullptr);
    ~CaseInformationDialog();

private slots:
    void setDateAndTime();
    void openKeyboardPhysicianName();
    void openKeyboardPatientId();
    void openKeyboardLocation();

    void on_pushButtonNext_clicked();

    void on_pushButtonPhysicianNameDown_clicked();

    void on_pushButtonLocationDown_clicked();

private:
    void enableNext(bool isNext);
    bool isFieldEmpty() const;

    Ui::CaseInformationDialog *ui;
    QDateTime m_now;
    QTimer m_displayTimer;
    SelectDialog* m_selectDialog{nullptr};
};

#endif // CASEINFORMATIONDIALOG_H
