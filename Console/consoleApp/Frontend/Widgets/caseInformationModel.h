#ifndef CASEINFORMATIONMODEL_H
#define CASEINFORMATIONMODEL_H

#include <QStringList>
#include <QString>
#include <set>

/*!
    \class CaseInformationModel saseInformationModel.h
    \brief The CaseInformationModel class implements the Case Information Model.

    The CaseInformationModel represnts the data shown and manipulated by the CaseInformationDialog
    The CaseInformationDialog class allows of editing of the physician names and locations
    The CaseInformationModel is an singleton available by name from any class
    The CaseInfornationModel is persisten - saved to a database
    \sa CaseInformationDialog
 */

using PhysicianNameContainer = std::set<QString>;

class CaseInformationModel
{
public:
    static CaseInformationModel* instance();

    void eraseLocations();
    void erasePhysicians();
    void initDefaults();
    /*!
     * \brief physicianNames - get the list of registered physicina names
     * \return the list of registered physician names
     */
    const PhysicianNameContainer &physicianNames() const;
    PhysicianNameContainer physicianNames2() const;

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
     * \brief addPhysicianName - add name to the registered physician names
     * \param name - the name to add to the registered physician names
     */
    void addPhysicianName(const QString& name);

    /*!
     * \brief setPhysicianName - set the physician name at index to name
     * \param index
     * \param name
     */
    void setPhysicianName(int index, const QString &name);

    /*!
     * \brief locations - get the list of registered locations
     * \return the list of registered locations
     */
    const PhysicianNameContainer& locations() const;
    PhysicianNameContainer locations2() const;

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
     * \brief addLocation - add location to the registered locations
     * \param location - the location to add to the registered locations
     */
    void addLocation(const QString& location);

    /*!
     * \brief setLocation - set the location at index to location
     * \param index
     * \param location
     */
    void setLocation(int index, const QString &location);

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
     * \brief resetModel remove selected physician name and selected location
     */
    void resetModel();

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

    QString defaultLocation() const;
    void setDefaultLocation(const QString &defaultLocation);

    QString defaultPhysicianName() const;
    void setDefaultPhysicianName(const QString &defaultPhysicianName);

    bool removePhysicianName(const QString& name);

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
     * \brief m_physicianNames - the pysician name registry
     */
    PhysicianNameContainer m_physicianNames; //{{"Dr. Himanshu Patel"}, {"Dr. Jaafer Golzar"}, {"Dr. Kara Parker-Smith"}};

    /*!
     * \brief m_selectedPhysicianName - the selected physician name for this case - empty string means no selection
     */
    QString m_selectedPhysicianName;

    /*!
     * \brief m_defaultPhysicianName - the default physician name
     */
    QString m_defaultPhysicianName;

    /*!
     * \brief m_locations - the location registry
     */
    PhysicianNameContainer m_locations; //{{"CATH LAB 1"}, {"CATH LAB 2"}, {"CATH LAB 3"}};

    /*!
     * \brief m_selectedLocation - the selected location for this case
     */
    QString m_selectedLocation;

    /*!
     * \brief m_selectedLocation - the selected location for this case
     */
    QString m_defaultLocation;

    /*!
     * \brief m_patientId - the patient id for this case
     */
    QString m_patientId;

    /*!
     * \brief m_dateAndTime - the date and time stamp of this case
     */
    QString m_dateAndTime;
};

#endif // CASEINFORMATIONMODEL_H
