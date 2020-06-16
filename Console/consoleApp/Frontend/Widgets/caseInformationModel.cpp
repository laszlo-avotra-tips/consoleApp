#include "caseInformationModel.h"

CaseInformationModel::CaseInformationModel()
{

}

QStringList CaseInformationModel::physicianNames() const
{
    return m_physicianNames;
}

QString CaseInformationModel::selectedPhysicianName() const
{
    return m_selectedPhysicianName;
}

void CaseInformationModel::setSelectedPhysicianName(const QString &selectedPysicianName)
{
    m_selectedPhysicianName = selectedPysicianName;
}

bool CaseInformationModel::isSelectedPhysicianName() const
{
    return !m_selectedPhysicianName.isEmpty();
}

void CaseInformationModel::addPhysicianName(const QString &name)
{
    m_physicianNames.push_back(name);
}

QStringList CaseInformationModel::locations() const
{
    return m_locations;
}

QString CaseInformationModel::selectedLocation() const
{
    return m_selectedLocation;
}

void CaseInformationModel::setSelectedLocation(const QString &selectedLocation)
{
    m_selectedLocation = selectedLocation;
}

QString CaseInformationModel::patientId() const
{
    return m_patientId;
}

void CaseInformationModel::setPatientId(const QString &patientId)
{
    m_patientId = patientId;
}

void CaseInformationModel::resetModel()
{
    m_selectedPhysicianName.clear();
    m_selectedLocation.clear();
    m_patientId.clear();
}

QString CaseInformationModel::dateAndTime() const
{
    return m_dateAndTime;
}

void CaseInformationModel::setDateAndTime(const QString &dateAndTime)
{
    m_dateAndTime = dateAndTime;
}
