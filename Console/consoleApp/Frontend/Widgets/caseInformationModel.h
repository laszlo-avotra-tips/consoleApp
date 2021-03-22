#ifndef CASEINFORMATIONMODEL_H
#define CASEINFORMATIONMODEL_H

#include <QStringList>
#include <QString>
#include <set>

/*!
    \class CaseInformationModel caseInformationModel.h
    \brief The CaseInformationModel class implements the Case Information Model.

    The CaseInformationModel represents the data shown and manipulated by the CaseInformationDialog
    The CaseInformationDialog class allows adding physician names and locations
    The CaseInformationModel is an singleton available by name from any class
    \sa CaseInformationDialog
 */

class PreferencesModel;
using PhysicianNameContainer = std::set<QString>;

class CaseInformationModel
{
public:
    static CaseInformationModel* instance();

    void init();

    /*!
     * \brief selectedPhysicianName - get the selected physician name
     * \return the selected physician name
     */
    QString selectedPhysicianName() const;

    /*!
     * \brief setSelectedPhysicianName - set the selected physician name
     * \param selectedPhysicianName - the selected physician name
     */
    void setSelectedPhysicianName(const QString &selectedPhysicianName);

    /*!
     * \brief isSelectedPhysicianName - is the selected physician name not empty
     * \return - true if the selected physician name is not empty, false otherwise
     */
    bool isSelectedPhysicianName() const;

    /*!
     * \brief selectedLocation - get the selected location
     * \return the selected location
     */
    QString selectedLocation() const;

    /*!
     * \brief setSelectedLocation - set the selected location
     * \param selectedLocation - the selected location
     */
    void setSelectedLocation(const QString &selectedLocation);

    /*!
     * \brief isSelectedLocation - is the selected location not empty
     * \return - true if the selected location is not empty, false otherwise
     */
    bool isSelectedLocation() const;

    /*!
     * \brief patientId - get the patient Id
     * \return - the patient Id
     */
    QString patientId() const;

    /*!
     * \brief setPatientId - set the patient Id to patientId
     * \param patientId - the patient Id
     */
    void setPatientId(const QString &patientId);

    /*!
     * \brief dateAndTime - get the case date and time stamp
     * \return - the case information date and time stamp
     */
    QString dateAndTime() const;

    /*!
     * \brief setDateAndTime - set the case date and time stamp
     * \param dateAndTime - the case date and time stamp
     */
    void setDateAndTime(const QString &dateAndTime);

    /*!
     * \brief validate - update with the database
     */
    void validate();

private:
    /*!
     * \brief CaseInformationModel - private constructor
     */
    CaseInformationModel();

    /*!
     * \brief m_instance - the static instance of this class
     */
    static CaseInformationModel* m_instance;

    /*!
     * \brief m_selectedPhysicianName - the selected physician name for this case - empty string means no selection
     */
    QString m_selectedPhysicianName;

    /*!
     * \brief m_selectedLocation - the selected location for this case
     */
    QString m_selectedLocation;

    /*!
     * \brief m_patientId - the patient id for this case
     */
    QString m_patientId;

    /*!
     * \brief m_dateAndTime - the date and time stamp of this case
     */
    QString m_dateAndTime;

    PreferencesModel* m_preferencesModel{nullptr};
};

#endif // CASEINFORMATIONMODEL_H
