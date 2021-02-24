#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QLabel>

#include <vector>

#include "activeLabel.h"

using PhysicansContainer = std::map<QString, ActiveLabel*>;

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
    std::vector<ActiveLabel*> m_labels;
    QString m_defaultPhysicianCandidate;

};

#endif // PREFERENCESDIALOG_H
