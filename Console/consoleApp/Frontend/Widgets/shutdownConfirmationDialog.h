#ifndef SHUTDOWNCONFIRMATIONDIALOG_H
#define SHUTDOWNCONFIRMATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ShutdownConfirmationDialog;
}

class ShutdownConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShutdownConfirmationDialog(QWidget *parent = nullptr);
    ~ShutdownConfirmationDialog();

private:
    Ui::ShutdownConfirmationDialog *ui;
};

#endif // SHUTDOWNCONFIRMATIONDIALOG_H
