#ifndef DEVICESELECTDIALOG_H
#define DEVICESELECTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QStringListModel>

#include "deviceListModel.h"

class MainScreen;
class DeviceDelegate;

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

private:
    void populateList(void);
    void initializeSelectedDevice();

private slots:
    void on_pushButtonDone_clicked();
    void startDaq(MainScreen *fe);

    void on_listViewAtherectomy_clicked(const QModelIndex &index);

    void on_listViewCto_clicked(const QModelIndex &index);

private:
    Ui::DeviceSelectDialog *ui;
    DeviceListModel* m_model{nullptr};
    DeviceListModel* m_ctoModel{nullptr};
    DeviceDelegate* m_delegate{nullptr};
};

#endif // DEVICESELECTDIALOG_H
