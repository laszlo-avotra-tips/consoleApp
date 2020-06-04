#ifndef DIALOGDEVICESELECT_H
#define DIALOGDEVICESELECT_H

#include <QDialog>
#include <QListWidgetItem>

class frontend;

namespace Ui {
class DialogDeviceSelect;
}

class DialogDeviceSelect : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDeviceSelect(QWidget *parent = nullptr);
    ~DialogDeviceSelect();

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
    Ui::DialogDeviceSelect *ui;
};

#endif // DIALOGDEVICESELECT_H
