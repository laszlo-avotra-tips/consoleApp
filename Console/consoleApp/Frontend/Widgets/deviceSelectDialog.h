#ifndef DEVICESELECTDIALOG_H
#define DEVICESELECTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

class frontend;
class QWidget;

namespace Ui {
class DeviceSelectDialog;
}

class DeviceSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSelectDialog(QWidget *parent = nullptr, const std::vector<QString> *param = nullptr);
    ~DeviceSelectDialog();

signals:
    void deviceSelected(int did);

protected:
    void changeEvent(QEvent *e);

private:
    void populateList(void);
    void populateList1(void);
    void initDialog( const std::vector<QString> *param = nullptr );

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
    void highlight(QWidget* label);
    void highlightCurrentDevice();
    int selectedDeviceId() const;
    void setSelectedDeviceId(int selectedDeviceId);

    Ui::DeviceSelectDialog *ui;
    int m_selectedDeviceId{-1};
    bool m_isAnimation{true};
};

#endif // DEVICESELECTDIALOG_H
