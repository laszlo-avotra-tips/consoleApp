#ifndef DIALOGCASEINFORMATION_H
#define DIALOGCASEINFORMATION_H

#include <QDialog>

namespace Ui {
class DialogCaseInformation;
}

class DialogCaseInformation : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCaseInformation(QWidget *parent = nullptr);
    ~DialogCaseInformation();

private:
    Ui::DialogCaseInformation *ui;
};

#endif // DIALOGCASEINFORMATION_H
