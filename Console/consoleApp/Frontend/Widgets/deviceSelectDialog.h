#ifndef DEVICESELECTDIALOG_H
#define DEVICESELECTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

class frontend;
class QLabel;

namespace Ui {
class DeviceSelectDialog;
}

class DeviceSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSelectDialog(QWidget *parent = nullptr);
    ~DeviceSelectDialog();

    void initDialog( void );
    bool isComplete() const;

signals:
    void deviceSelected(int did);

protected:
    void changeEvent(QEvent *e);

private:
    void populateList(void);
    void populateList1(void);

private slots:
    void on_pushButtonDone_clicked();
    void on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item);
    void startDaq(frontend *fe);
    void handleDevice0();
    void handleDevice1();
    void handleDevice2();
    void handleDeviceSelected(int did);

    void on_listWidgetAtherectomy_clicked(const QModelIndex &index);

private:
    void removeHighlight();
    void highlight(QLabel* label);

    Ui::DeviceSelectDialog *ui;
};

#endif // DEVICESELECTDIALOG_H
