#ifndef CASEINFORMATIONDIALOG_H
#define CASEINFORMATIONDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>

class SelectDialog;
class CaseInformationModel;

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
    \sa WidgetContainer DialogFactory CaseInformationModel
 */

class CaseInformationDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief constructor
     * \param parent - the parent controls the instances lifetime
     */
    explicit CaseInformationDialog(QWidget *parent = nullptr);

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
     * \brief openKeyboardPhysicianName - open the keyboard to edit the physicians name
     */
    void openKeyboardPhysicianName();

    /*!
     * \brief openKeyboardPatientId - open the keyboard to edit the patient ID
     */
    void openKeyboardPatientId();

    /*!
     * \brief openKeyboardLocation - open the keyboard to edit the location
     */
    void openKeyboardLocation();

    void handleNext();
    void handlePhysicianNameSelect();
    void handleLocationSelect();
    void handleBack();
    void initDialog();

private:
    void enableNext(bool isNext);

    Ui::CaseInformationDialog *ui;
    QDateTime m_now;
    QTimer m_displayTimer;
    SelectDialog* m_selectDialog{nullptr};

    CaseInformationModel& m_model;
};

#endif // CASEINFORMATIONDIALOG_H
