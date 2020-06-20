#ifndef EMPTYDIALOG_H
#define EMPTYDIALOG_H

#include <QDialog>

namespace Ui {
class EmptyDialog;
}

class EmptyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmptyDialog(QWidget *parent = nullptr);
    ~EmptyDialog();

private:
    Ui::EmptyDialog *ui;
};

#endif // EMPTYDIALOG_H
