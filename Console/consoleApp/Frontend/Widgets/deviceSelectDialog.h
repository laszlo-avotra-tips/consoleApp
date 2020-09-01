#ifndef DEVICESELECTDIALOG_H
#define DEVICESELECTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QStringListModel>

#include "deviceListModel.h"

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

    void initDialog( void );
    bool isComplete() const;

private:
    void populateList(void);

private slots:
    void on_pushButtonDone_clicked();
    void startDaq(frontend *fe);

    void on_listViewAtherectomy_clicked(const QModelIndex &index);

    void on_listViewCto_clicked(const QModelIndex &index);

private:
    Ui::DeviceSelectDialog *ui;
    DeviceListModel* m_model{nullptr};
    DeviceListModel* m_ctoModel{nullptr};
};

#endif // DEVICESELECTDIALOG_H
