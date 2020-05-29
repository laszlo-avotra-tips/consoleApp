#ifndef DIALOGDEVICESELECT_H
#define DIALOGDEVICESELECT_H

#include <QDialog>

namespace Ui {
class DialogDeviceSelect;
}

class DialogDeviceSelect : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDeviceSelect(QWidget *parent = nullptr);
    ~DialogDeviceSelect();

private:
    Ui::DialogDeviceSelect *ui;
};

#endif // DIALOGDEVICESELECT_H
