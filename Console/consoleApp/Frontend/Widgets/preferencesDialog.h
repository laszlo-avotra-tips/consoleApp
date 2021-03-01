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
    void handleSelectedLocation(const QString &name);

    void setDefaultPhysician();
    void setDefaultLocation();

    void persistPreferences();

    void handleAddRemoveLocation();
    void handleAddRemovePhysician();
    void handleAddLocation();
    void handleAddPhysician();
    void handleRemoveLocation();
    void handleRemovePhysician();

    void handlePhysicianUp();
    void handlePhysicianDown();

    void handleLocationUp();
    void handleLocationDown();
private:
    void initPhysiciansContainer();
    void initLocationsContainer();
    void initPhysiciansContainer2();
    void initLocationsContainer2();
    void createCaseInfoDb();

    Ui::PreferencesDialog *ui;
    LabelContainer m_physiciansContainer;
    LabelContainer m_locationsContainer;

    std::vector<QLabel*> m_physicianLabels;
    std::vector<QLabel*> m_locationLabels;

    QString m_defaultPhysicianCandidate;
    QString m_defaultLocationCandidate;

    QLabel* m_selectedPhysicianLabel{nullptr};
    QLabel* m_selectedLocationLabel{nullptr};

    int m_physicianBase{0};
    int m_locationBase{0};
};

#endif // PREFERENCESDIALOG_H
