#include "displayManager.h"
#include "logger.h"
#include "formSecondMonitor.h"
#include "livescene.h"
#include "formPmLogo.h"

#include <QFileSystemWatcher>
#include <QProcess>
#include <QGraphicsScene>
#include <QGraphicsView>

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
    LOG0
    m_diplaySettingsMonitor->kill();
    QThread::msleep(1000);
    delete this;
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

void DisplayManager::showSecondMonitor(bool isNonPrimaryMonitorPresent)
{
    LOG1(isNonPrimaryMonitorPresent)
    if(isNonPrimaryMonitorPresent){
        m_physicianMonitor->show();
        m_physicianMonitor->move(3240,0);
        m_physicianMonitor->showFullScreen();
    } else {
        m_physicianMonitor->hide();
    }

}

bool DisplayManager::isNonPrimaryMonitorPresent() const
{
    return m_isNonPrimaryMonitorPresent;
}

void DisplayManager::setIsNonPrimaryMonitorPresent(bool isNonPrimaryMonitorPresent)
{
    m_isNonPrimaryMonitorPresent = isNonPrimaryMonitorPresent;
}

void DisplayManager::setScene(liveScene *scene)
{
    m_secondMonitor->setScene(scene);
}

DisplayManager::DisplayManager(QObject *parent) : QObject(parent)
{
    m_eventFileWatcher = std::make_unique< QFileSystemWatcher>();
    m_eventFileWatcher->addPath(m_eventFileName);

    connect(m_eventFileWatcher.get(), &QFileSystemWatcher::fileChanged, this, &DisplayManager::monitorEvent);

    m_diplaySettingsMonitor = std::make_unique<QProcess>();

    m_diplaySettingsMonitor->setArguments(m_programArguments);
    m_diplaySettingsMonitor->setProgram(m_programName);
    m_diplaySettingsMonitor->start();

    m_secondMonitor = std::make_unique<FormSecondMonitor>();
    m_pmLogo = std::make_unique<FormPmLogo>();

    m_physicianMonitor = m_secondMonitor.get();

    connect(this, &DisplayManager::nonPrimaryMonitorIsPresent, this, &DisplayManager::showSecondMonitor);

    m_physicianMonitor->hide();

    connect(    m_diplaySettingsMonitor.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &DisplayManager::programFinished);

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

void DisplayManager::programFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    const std::map<QProcess::ExitStatus,QString>
    lut{
        {{QProcess::NormalExit},{"NormalExit"}},
        {{QProcess::CrashExit},{"CrashExit"}}
    };

    QString message;
    const auto it = lut.find(exitStatus);
    if(it != lut.end()){
        message = it->second;
    }else {
        message = QString::number(int(exitStatus));
    }

    LOG2(exitCode, message)
}
