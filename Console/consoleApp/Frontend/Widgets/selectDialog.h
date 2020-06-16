#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include "consoleLineEdit.h"

#include <QDialog>
#include <QTimer>
#include <QString>

#include <vector>

class QStringList;

using SelectableWidgetContainer = std::vector<ConsoleLineEdit*>;

namespace Ui {
class SelectDialog;
}

class SelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectDialog(QWidget *parent = nullptr);
    ~SelectDialog();

    void populate(const QStringList& sl);

    QString selectedItem() const;

private slots:
    void selectItem0();
    void selectItem1();
    void selectItem2();

private:
    void selectItem(int index);

    Ui::SelectDialog *ui;
    SelectableWidgetContainer m_selectableWidgets;
    QString m_selectedItem;
};

#endif // SELECTDIALOG_H
