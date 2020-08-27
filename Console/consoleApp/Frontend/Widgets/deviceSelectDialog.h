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
    void populateList2(void);

private slots:
    void on_pushButtonDone_clicked();
    void on_listWidgetAtherectomy_itemClicked(QListWidgetItem *item);
    void startDaq(frontend *fe);

    void on_listWidgetAtherectomy_clicked(const QModelIndex &index);

    void on_listViewAtherectomy_clicked(const QModelIndex &index);

private:
    Ui::DeviceSelectDialog *ui;
    DeviceListModel* m_model2{nullptr};
};

#endif // DEVICESELECTDIALOG_H
