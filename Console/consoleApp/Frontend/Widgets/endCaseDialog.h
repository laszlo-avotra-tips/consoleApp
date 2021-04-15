#ifndef ENDCASEDIALOG_H
#define ENDCASEDIALOG_H

#include <QDialog>

namespace Ui {
class endCaseDialog;
}

class endCaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit endCaseDialog(QWidget *parent = nullptr);
    ~endCaseDialog();

private:
    Ui::endCaseDialog *ui;
};

#endif // ENDCASEDIALOG_H
