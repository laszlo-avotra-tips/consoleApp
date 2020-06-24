#ifndef CASEINFORMATIONDIALOG_H
#define CASEINFORMATIONDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>

class SelectDialog;
class CaseInformationModel;

namespace Ui {
class CaseInformationDialog;
}

class CaseInformationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaseInformationDialog(QWidget *parent = nullptr);
    ~CaseInformationDialog();

    static void reset();

private slots:
    void setDateAndTime();
    void openKeyboardPhysicianName();
    void openKeyboardPatientId();
    void openKeyboardLocation();

    void on_pushButtonNext_clicked();
    void on_pushButtonPhysicianNameDown_clicked();
    void on_pushButtonLocationDown_clicked();
    void initDialog();
    void handleBack();

private:
    void enableNext(bool isNext);

    Ui::CaseInformationDialog *ui;
    QDateTime m_now;
    QTimer m_displayTimer;
    SelectDialog* m_selectDialog{nullptr};

    CaseInformationModel& m_model;
};

#endif // CASEINFORMATIONDIALOG_H
