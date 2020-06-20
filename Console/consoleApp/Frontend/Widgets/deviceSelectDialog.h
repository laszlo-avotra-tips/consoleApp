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

signals:
    void deviceSelected(int did);

protected:
    void changeEvent(QEvent *e);

private:
    void populateList(void);
    void populateList1(void);

private slots:
    void on_pushButtonDone_clicked();
    void startDaq(frontend *fe);
    void handleDevice0();
    void handleDevice1();
    void handleDevice2();
    void handleDevice3();
    void handleDeviceSelected(int did);

private:
    void removeHighlight();
    void highlight(QLabel* label);

    Ui::DeviceSelectDialog *ui;
};

#endif // DEVICESELECTDIALOG_H
