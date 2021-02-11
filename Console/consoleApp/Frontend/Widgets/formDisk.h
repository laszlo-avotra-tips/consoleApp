#ifndef FORMDISK_H
#define FORMDISK_H

#include <QWidget>

namespace Ui {
class FormDisk;
}

class FormDisk : public QWidget
{
    Q_OBJECT

public:
    explicit FormDisk(QWidget *parent = nullptr);
    ~FormDisk();
    void setRuntimeLabel(const QString &msg);
    void setCurrentTime(const QString &msg);

private slots:
    void setInfoLabel(const QString& msg);

private:
    Ui::FormDisk *ui;
};

#endif // FORMDISK_H
