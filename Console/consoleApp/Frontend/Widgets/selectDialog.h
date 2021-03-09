#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include "consoleLineEdit.h"

#include <QDialog>
#include <QTimer>
#include <QString>

#include <vector>
#include "caseInformationDialog.h"


using SelectableWidgetContainer = std::vector<ConsoleLineEdit*>;

namespace Ui {
class SelectDialog;
}

class PreferencesModel;

class SelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectDialog(const QString& name, CaseInformationDialog *parent = nullptr);
    ~SelectDialog();

    void initializeSelect(const PhysicianNameContainer& sl, QString selected);

    QString selectedItem() const;

public slots:
    void closeDialog(bool isChecked);

private slots:
    void selectItem0();
    void selectItem1();
    void selectItem2();
    void scrollDown();
    void addNew();
    void highlight(const QString& selected);

private:
    void selectItem(int index);
    int indexOf(const PhysicianNameContainer& cont, const QString& val) const;
    void incrementCircular(const PhysicianNameContainer& cont, PhysicianNameContainer::iterator& it);
    void decrementCircular(const PhysicianNameContainer& cont, PhysicianNameContainer::iterator& it);
    void populateItemsInview(const QString& selected);

    QString m_name;
    Ui::SelectDialog *ui;
    SelectableWidgetContainer m_selectableWidgets;
    PhysicianNameContainer m_items;
    QStringList m_itemsInView;
    QString m_selectedItem;
    CaseInformationDialog* m_parent{nullptr};
    PreferencesModel* m_pModel{nullptr};
};

#endif // SELECTDIALOG_H
