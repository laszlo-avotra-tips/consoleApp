#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QLabel>

#include <vector>

#include "activeLabel.h"

using LabelContainer = std::map<QString, QLabel*>;

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
    void handleSelectedPhysician(const QString &name);
    void setDefaultPhysician();
    void handleSelectedLocation(const QString &name);
    void setDefaultLocation();

private:
    void initPhysiciansContainer();
    void initLocationsContainer();

    Ui::PreferencesDialog *ui;
    LabelContainer m_physiciansContainer;
    LabelContainer m_locationssContainer;

    std::vector<QLabel*> m_physicianLabels;
    std::vector<QLabel*> m_locationLabels;

    QString m_defaultPhysicianCandidate;
    QString m_defaultLocationCandidate;

    QLabel* m_selectedPhysicianLabel{nullptr};
    QLabel* m_selectedLocationLabel{nullptr};
};

#endif // PREFERENCESDIALOG_H
