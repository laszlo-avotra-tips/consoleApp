#include "caseInformationModel.h"
#include "Utility/sessiondatabase.h"
#include "Utility/userSettings.h"
#include "util.h"
#include "logger.h"
#include "fullCaseRecorder.h"
#include "Utility/caseInfoDatabase.h"
#include <algorithm>

CaseInformationModel* CaseInformationModel::m_instance{nullptr};

CaseInformationModel::CaseInformationModel()
{
}

void CaseInformationModel::initDefaults()
{
    const auto& settings = userSettings::Instance();
    const auto& defaultPhysicianName = settings.getPhysician();
    auto itPhn = std::find(m_physicianNames.begin(), m_physicianNames.end(), defaultPhysicianName);
    if(itPhn != m_physicianNames.end())
    {
        m_defaultPhysicianName = defaultPhysicianName;
    }
    const auto& defaultLocation = settings.getLocation();
    auto itL = std::find(m_locations.begin(), m_locations.end(), defaultLocation);
    if(itL != m_locations.end())
    {
        m_defaultLocation = defaultLocation;
    }
}

QString CaseInformationModel::defaultPhysicianName() const
{
    return m_defaultPhysicianName;
}

void CaseInformationModel::setDefaultPhysicianName(const QString &defaultPhysicianName)
{
    m_defaultPhysicianName = defaultPhysicianName;
}

bool CaseInformationModel::removePhysicianName(const QString &name)
{
    bool success{false};
    size_t size{};
    if(name != m_selectedPhysicianName){
        size = m_physicianNames.erase(name);
        success = true;
        CaseInfoDatabase ciDb;
        const auto& ci = CaseInformationModel::instance();
        const auto& names = ci->physicianNames();
        ciDb.updatePhysicianTable(names);
    }
    return success;
}

bool CaseInformationModel::removeLocation(const QString &name)
{
    bool success{false};
    size_t size{};
    if(name != m_selectedLocation){
        size = m_locations.erase(name);
        success = true;
        CaseInfoDatabase ciDb;
        const auto& ci = CaseInformationModel::instance();
        const auto& names = ci->locations();
        ciDb.updateLocationTable(names);
    }
    return success;
}

void CaseInformationModel::persistModel()
{
    CaseInfoDatabase ciDb;
    ciDb.initCaseInfo();

    QSqlQuery q;

    q.prepare( QString("DELETE FROM Physicians"));
    q.exec();
    QSqlError sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }

    q.prepare( QString("DELETE FROM Locations"));
    q.exec();
    sqlerr = q.lastError();
    if(sqlerr.isValid()){
        const QString& errorMsg = sqlerr.databaseText();
        LOG1(errorMsg)
    }

    for(const auto& physician : physicianNames()){
        ciDb.addPhysician(physician);
    }

    for(const auto& location : locations()){
        ciDb.addLocation(location);
    }
}

QString CaseInformationModel::defaultLocation() const
{
    return m_defaultLocation;
}

void CaseInformationModel::setDefaultLocation(const QString &defaultLocation)
{
    m_defaultLocation = defaultLocation;
}

CaseInformationModel *CaseInformationModel::instance()
{
    if(!m_instance){
        m_instance = new CaseInformationModel();
    }
    return m_instance;
}

void CaseInformationModel::eraseLocations()
{
    m_locations.erase(m_locations.begin(), m_locations.end());
}

void CaseInformationModel::erasePhysicians()
{
    m_physicianNames.erase(m_physicianNames.begin(), m_physicianNames.end());
}

const PhysicianNameContainer& CaseInformationModel::physicianNames() const
{
    return m_physicianNames;
}

PhysicianNameContainer CaseInformationModel::physicianNames2() const
{
    return PhysicianNameContainer{{"Dr. Himanshu Patel"}, {"Dr. Jaafer Golzar"}, {"Dr. Kara Parker-Smith"}};
}

QString CaseInformationModel::selectedPhysicianName() const
{
    return m_selectedPhysicianName;
}

void CaseInformationModel::setSelectedPhysicianName(const QString &selectedPysicianName)
{
    LOG1(selectedPysicianName)
    m_selectedPhysicianName = selectedPysicianName;
}

bool CaseInformationModel::isSelectedPhysicianName() const
{
    return !m_selectedPhysicianName.isEmpty();
}

void CaseInformationModel::addPhysicianName(const QString &name)
{
    m_physicianNames.insert(name);
}

void CaseInformationModel::setPhysicianName(int index, const QString &name)
{
    auto old = m_physicianNames.begin();
    for(int i = 0; i < index; ++i){
        ++old;
    }
    m_physicianNames.erase(old);
    m_physicianNames.insert(name);
}

const PhysicianNameContainer& CaseInformationModel::locations() const
{
    return m_locations;
}

PhysicianNameContainer CaseInformationModel::locations2() const
{
    return PhysicianNameContainer{{"CATH LAB 1"}, {"CATH LAB 2"}, {"CATH LAB 3"}};
}

QString CaseInformationModel::selectedLocation() const
{
    return m_selectedLocation;
}

void CaseInformationModel::setSelectedLocation(const QString &selectedLocation)
{
    m_selectedLocation = selectedLocation;
}

bool CaseInformationModel::isSelectedLocation() const
{
    return !m_selectedLocation.isEmpty();
}

void CaseInformationModel::addLocation(const QString &location)
{
    m_locations.insert(location);
}

void CaseInformationModel::setLocation(int index, const QString &location)
{
    auto old = m_locations.begin();
    for(int i = 0; i < index; ++i){
        ++old;
    }
    m_locations.erase(old);
    m_locations.insert(location);
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

void CaseInformationModel::validate()
{
    caseInfo &info      = caseInfo::Instance();
    sessionDatabase db;

    /*
     * Set the case information
     **/
    info.setPatientID( m_patientId );
    info.setDoctor( m_selectedPhysicianName);
    info.setLocation( m_selectedLocation);

    /*
     * Create the session directory
     */
    QDir dir;

    /*
     *  set defaults
     */
    info.setUtcOffset( 0 );

    /*
     * Create a unique case storage container.  By convention GUID's
     * have curly brackets around them; however, that causes mkisofs
     * to complain when exporting so they are removed for our use.
     */
    QString uuid = QUuid::createUuid().toString();
    uuid.remove( "{" ).remove( "}" );
    info.setCaseID( uuid );

//    FullCaseRecorder::instance()->setFullCaseDir(info.getFullCaseDir());
//    FullCaseRecorder::instance()->startRecording();

    /*
     *  Create the case session directories.
     */
    if ( dir.mkdir( info.getStorageDir()  ) &&
        dir.mkdir( info.getClipsDir()    ) &&
        dir.mkdir( info.getCapturesDir() ) &&
        dir.mkdir( info.getFullCaseDir() ) )
    {
        /*
         * the session directory structure was successfully created
         */

        /*
         *  determine how far off of UTC we are
         */
        QDateTime now = QDateTime::currentDateTime();

        /*
         *  check if we need to account for UTC being tomorrow relative to us
         */
        int dayOffset = 0;
        if( now.date() < now.toUTC().date() )
        {
            dayOffset = 24;
        }

        info.setUtcOffset( now.time().hour() - ( now.toUTC().time().hour() + dayOffset ) );

        /*
         *  Create and save the session information to the case database
         */
        db.initSessionDb();
        db.createSession();

        /*
         *  save a cookie for HomeScreen to find
         */
        updateSessionCookieFile( info.getCaseID() );

        LOG( INFO, QString( "Case ID: %1" ).arg( info.getCaseID() ) )
    }
}
