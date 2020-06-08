#ifndef DIALOGCASEINFORMATION_H
#define DIALOGCASEINFORMATION_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>

namespace Ui {
class DialogCaseInformation;
}

class DialogCaseInformation : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCaseInformation(QWidget *parent = nullptr);
    ~DialogCaseInformation();

private slots:
    void setDateAndTime();
    void openKeyboardPhysicianName();
    void openKeyboardPatientId();
    void openKeyboardLocation();

private:
    Ui::DialogCaseInformation *ui;
    QDateTime m_now;
    QTimer m_displayTimer;
};

#endif // DIALOGCASEINFORMATION_H
