#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include <vector>

#include "activeLabel.h"
#include "Utility/caseInfoDatabase.h"

using LabelContainer = std::map<QString, ActiveLabel*>;

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
    void handleLabelDr1(ActiveLabel* label);
    void handleLabelDr2(ActiveLabel* label);
    void handleLabelDr3(ActiveLabel* label);

    void handleLocation1(ActiveLabel* label);
    void handleLocation2(ActiveLabel* label);
    void handleLocation3(ActiveLabel* label);

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
    void updatePysicianLabels();
    void updateLocationLabels();
    void unmarkAll(std::vector<ActiveLabel*>& container);

    Ui::PreferencesDialog *ui;
    LabelContainer m_physiciansContainer;
    LabelContainer m_locationsContainer;

    std::vector<ActiveLabel*> m_physicianLabels;
    std::vector<ActiveLabel*> m_locationLabels;

    QString m_defaultPhysicianCandidate;
    QString m_defaultLocationCandidate;

    QString m_highlightedPhysician;
    QString m_highlightedLocation;

    bool m_isCandidatePhysitian{false};
    bool m_isCandidateLocation{false};

    ActiveLabel* m_selectedPhysicianLabel{nullptr};
    ActiveLabel* m_selectedLocationLabel{nullptr};

    int m_physicianBase{0};
    int m_locationBase{0};

    PhysicianNameContainer::iterator m_phIt;
    PhysicianNameContainer::iterator m_locIt;

};

#endif // PREFERENCESDIALOG_H
