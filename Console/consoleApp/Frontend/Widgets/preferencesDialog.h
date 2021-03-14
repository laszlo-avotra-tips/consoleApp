#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QTimer>

#include <vector>

#include "activeLabel.h"
#include "Utility/preferencesModel.h"

using LabelContainer = std::map<QString, ActiveLabel*>;

namespace Ui {
class PreferencesDialog;
}

class DateTimeController;

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

    void handlePhysicianUnmarked();
    void handleLocationUnmarked();

    void setCurrentDate();
    void setCurrentTime();

private:
    void handlePhysicianLabel(const ActiveLabel* label);
    void handleLocationLabel(const ActiveLabel* label);
    void showEvent(QShowEvent* se) override;
    void hideEvent(QHideEvent* se) override;

    void initPhysiciansContainer();
    void initLocationsContainer();
    void init();
    void updatePysicianLabels();
    void updateLocationLabels();
    void unmarkAll(std::vector<ActiveLabel*>& container);
    void markCandidate(std::vector<ActiveLabel*>& cont, const QString& name);


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

    PreferenceContainers_t::iterator m_phIt;
    PreferenceContainers_t::iterator m_locIt;

    PreferencesModel* m_model{nullptr};

    QTimer m_currentTimeUpdate;
    QTimer m_currentDateUpdate;

    DateTimeController* m_dateTimeController{nullptr};

};

#endif // PREFERENCESDIALOG_H
