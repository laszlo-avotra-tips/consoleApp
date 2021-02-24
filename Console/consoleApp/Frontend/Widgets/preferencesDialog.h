#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QLabel>

#include <vector>

#include "activeLabel.h"

using PhysicansContainer = std::map<QString, QLabel*>;

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

private slots:
    void handleSelectedLabel(const QString &name);
    void setDefaultPhysician();

private:
    void initPhysiciansContainer();

    Ui::PreferencesDialog *ui;
    PhysicansContainer m_physiciansContainer;
    std::vector<QLabel*> m_labels;
    QString m_defaultPhysicianCandidate;
    QLabel* m_selectedLabel{nullptr};
};

#endif // PREFERENCESDIALOG_H
