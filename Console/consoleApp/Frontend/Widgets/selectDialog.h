#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include "consoleLineEdit.h"

#include <QDialog>
#include <QTimer>
#include <QString>

#include <vector>
#include <caseInformationModel.h>


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

    void populate(const PhysicianNameContainer& sl, const QString& selected);

    QString selectedItem() const;

private slots:
    void selectItem0();
    void selectItem1();
    void selectItem2();
    void scrollDown();

private:
    void selectItem(int index);
    int indexOf(const PhysicianNameContainer& cont, const QString& val) const;
    void incrementCircular(const PhysicianNameContainer& cont, PhysicianNameContainer::iterator& it);
    void decrementCircular(const PhysicianNameContainer& cont, PhysicianNameContainer::iterator& it);

    Ui::SelectDialog *ui;
    SelectableWidgetContainer m_selectableWidgets;
    PhysicianNameContainer m_items;
    PhysicianNameContainer m_itemsInView;
    QString m_selectedItem;
};

#endif // SELECTDIALOG_H
