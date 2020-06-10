#ifndef DEVICESELECTDIALOG_H
#define DEVICESELECTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

class frontend;

namespace Ui {
class DeviceSelectDialog;
}

class DeviceSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSelectDialog(QWidget *parent = nullptr);
    ~DeviceSelectDialog();

    void init( void );
    bool isComplete() const;

protected:
    void changeEvent(QEvent *e);

private:
    void populateList(void);

private slots:
    void on_pushButtonDone_clicked();
    void on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item);
    void startDaq(frontend *fe);

    void on_listWidgetAtherectomy_clicked(const QModelIndex &index);

private:
    Ui::DeviceSelectDialog *ui;
};

#endif // DEVICESELECTDIALOG_H
