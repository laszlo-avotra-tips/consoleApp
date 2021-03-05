#ifndef CASEINFORMATIONDIALOG_H
#define CASEINFORMATIONDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>
#include "caseInformationModel.h"

class SelectDialog;

namespace Ui {
class CaseInformationDialog;
}

/*!
    \class CaseInformationDialog caseInformationDialog.h
    \brief The CaseInformationDialog class implements the Case Information Dialog of the UI

    The CaseInformationDialog class provides access to the database with configured physician
    names and locations.
    The CaseInformationDialog class allows of editing of the physician names and locations
    in the database.
    The CaseInformationDialog class can be used to add physician names and locations to the
    database.
    The CaseInformationDialog class can be used to assign a case ID.
    The CaseInformationDialog class assigns a date and time stamp to the case.
    The CaseInformationDialog is instantiated througt the widget container.
    \sa WidgetContainer DialogFactory CaseInformationModel SelectDialog ConsoleLineEdit ConsoleKeyboard
 */

class CaseInformationDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief constructor
     * \param parent - the parent controls the instances lifetime
     * \param param - list of constructor parameters
     */
    explicit CaseInformationDialog(QWidget *parent = nullptr, const std::vector<QString> *param = nullptr);

    /*!
     * \brief destructor
     */
    ~CaseInformationDialog();

    /*!
     * \brief reset the case information model to default values
     */
    static void reset();

private slots:
    /*!
     * \brief setDateAndTime - store the date and time for this case
     */
    void setDateAndTime();

    /*!
     * \brief editOrSelectPhysicianName - edit or select physician name
     */
    void editOrSelectPhysicianName();

    /*!
     * \brief openKeyboardPatientId - open the keyboard to edit the patient ID
     */
    void openKeyboardPatientId();

    /*!
     * \brief openKeyboardLocation - open the keyboard to edit the location
     */
    void editOrSelectLocation();

    /*!
     * \brief handleNext
     */
    void handleNext();

    /*!
     * \brief handlePhysicianNameSelect
     * \param isCkecked
     */
    void handlePhysicianNameSelect(bool isChecked);

    /*!
     * \brief handleLocationSelect
     */
    void handleLocationSelect();

    /*!
     * \brief handleBack
     */
    void handleBack();

    /*!
     * \brief initDialog
     * \param param - list of constructor parameters
     */
    void initDialog(const std::vector<QString> *param = nullptr);

    int indexOf(const PhysicianNameContainer& cont, const QString& val) const;

private:
    /*!
     * \brief showEvent
     * \param se
     */
    void showEvent(QShowEvent* se) override;

    /*!
     * \brief enableNext - enable/disable the < NEXT> > button
     * \param isNext - enable if true, disable otherwise
     */
    void enableNext(bool isNext);

    /*!
     * the widgets
     */
    Ui::CaseInformationDialog *ui;

    /*!
     * the date and time
     */
    QDateTime m_now;

    /*!
     * the timer that updates the date and time display
     */
    QTimer m_displayTimer;

    /*!
     * select dialog - used for selecting or adding a physician or a location
     */
    SelectDialog* m_selectDialog{nullptr};

    /*!
     * the model
     */
    CaseInformationModel& m_model;

    /*!
     * \brief m_isNewCase - true for new case
     */
    bool m_isNewCase{true};
};

#endif // CASEINFORMATIONDIALOG_H
