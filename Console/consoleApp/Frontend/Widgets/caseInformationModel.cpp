#include "caseInformationModel.h"
#include "Utility/sessiondatabase.h"
#include "Utility/userSettings.h"
#include "util.h"
#include "logger.h"
#include "fullCaseRecorder.h"

CaseInformationModel* CaseInformationModel::m_instance{nullptr};

CaseInformationModel::CaseInformationModel()
{
    m_physicianNames = userSettings::Instance().getPhysicians();
    m_locations = userSettings::Instance().getLocations();
}

CaseInformationModel *CaseInformationModel::instance()
{
    if(!m_instance){
        m_instance = new CaseInformationModel();
    }
    return m_instance;
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

void CaseInformationModel::setPhysicianName(int index, const QString &name)
{
    m_physicianNames[index] = name;
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

bool CaseInformationModel::isSelectedLocation() const
{
    return !m_selectedLocation.isEmpty();
}

void CaseInformationModel::addLocation(const QString &location)
{
    m_locations.push_back(location);
}

void CaseInformationModel::setLocation(int index, const QString &location)
{
    m_locations[index] = location;
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
        db.initDb();
        db.createSession();

        /*
         *  save a cookie for HomeScreen to find
         */
        updateSessionCookieFile( info.getCaseID() );

        LOG( INFO, QString( "Case ID: %1" ).arg( info.getCaseID() ) )
    }
}
