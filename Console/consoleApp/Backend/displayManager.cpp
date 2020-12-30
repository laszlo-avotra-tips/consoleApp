#include "displayManager.h"
#include "logger.h"

#include <QFileSystemWatcher>
#include <QProcess>

DisplayManager* DisplayManager::m_instance{nullptr};

DisplayManager *DisplayManager::instance()
{
    if(!m_instance){
        m_instance = new DisplayManager();
    }
    return m_instance;
}

void DisplayManager::killDisplayMonitor()
{
    m_diplaySettingsMonitor->kill();
}

QString DisplayManager::eventFileName() const
{
    return m_eventFileName;
}

void DisplayManager::monitorEvent(const QString &fileName)
{
    LOG1(fileName)
    parseEventFile(fileName);
}

bool DisplayManager::isNonPrimaryMonitorPresent() const
{
    return m_isNonPrimaryMonitorPresent;
}

void DisplayManager::setIsNonPrimaryMonitorPresent(bool isNonPrimaryMonitorPresent)
{
    m_isNonPrimaryMonitorPresent = isNonPrimaryMonitorPresent;
}

DisplayManager::DisplayManager(QObject *parent) : QObject(parent)
{
    //    m_eventFileWatcher = new QFileSystemWatcher(QStringList(R"(C:\Avinger_System)"));
    m_eventFileWatcher = new QFileSystemWatcher();
    m_eventFileWatcher->addPath(m_eventFileName);

    connect(m_eventFileWatcher, &QFileSystemWatcher::fileChanged, this, &DisplayManager::monitorEvent);

    m_diplaySettingsMonitor = new QProcess();

    m_diplaySettingsMonitor->setArguments(m_programArguments);
    m_diplaySettingsMonitor->setProgram(m_programName);
    m_diplaySettingsMonitor->start();

}

void DisplayManager::parseEventFile(const QString &eventFileName)
{
    const bool wasNonPrimaryPresent = m_isNonPrimaryMonitorPresent;
    m_isNonPrimaryMonitorPresent = false;
    QFile eventFile(eventFileName);
    eventFile.open(QFile::ReadOnly);
    QTextStream eventText(&eventFile);
    int lines{0};

    while(!eventText.atEnd()){
        QString line = eventText.readLine();
        if(!line.isEmpty()){
            ++lines;
            parse(line);
        }
    }
    eventFile.close();
    LOG2(wasNonPrimaryPresent, m_isNonPrimaryMonitorPresent)
    if(isNonPrimaryMonitorPresent() != wasNonPrimaryPresent){
        emit nonPrimaryMonitorIsPresent(isNonPrimaryMonitorPresent());
    }
}

void DisplayManager::parse(const QString &line)
{
    if(line.startsWith("Primary monitor:")){
       if(line.endsWith("No")){
           m_isNonPrimaryMonitorPresent = true;
       }
    }
}
