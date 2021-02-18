#include "displayManager.h"
#include "logger.h"
#include "formSecondMonitor.h"
#include "livescene.h"
#include "formPmLogo.h"
#include "opaqueScreen.h"
#include "formDisk.h"
#include "Utility/userSettings.h"

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

void DisplayManager::showHideSecondMonitor(bool isNonPrimaryMonitorPresent)
{
    LOG1(isNonPrimaryMonitorPresent)
    if(isNonPrimaryMonitorPresent){
        m_widgetOnTheSecondMonitor->show();
        m_widgetOnTheSecondMonitor->move(ControlScreenWidth,0);
        m_widgetOnTheSecondMonitor->showFullScreen();
    } else {
        m_widgetOnTheSecondMonitor->hide();
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
    m_liveSceneView->setScene(scene);
}

void DisplayManager::initWidgetForTheSecondMonitor(QString name)
{
    LOG1(name);

    auto it = m_widgetContainer.find(name);
    if(it != m_widgetContainer.end()){
        m_widgetOnTheSecondMonitor->hide();
        //find widget by name assign to m_widgetOnTheSecondMonitor
        if(it->second){
            m_widgetOnTheSecondMonitor = it->second;
            m_widgetOnTheSecondMonitor->setWindowFlags(Qt::SplashScreen);
            m_widgetOnTheSecondMonitor->move(ControlScreenWidth,0);
        }
    }
    emit nonPrimaryMonitorIsPresent(isNonPrimaryMonitorPresent());
}

void DisplayManager::initSecondMonitor(QString name)
{
    auto it = m_widgetContainer.find(name);
    if(it != m_widgetContainer.end()){
        if(it->second){
            m_widgetOnTheSecondMonitor = it->second;
            m_widgetOnTheSecondMonitor->setWindowFlags(Qt::SplashScreen);
            m_widgetOnTheSecondMonitor->move(ControlScreenWidth,0);
            LOG1(name)
        }
    }
}

void DisplayManager::setWindowTitle(const QString &msg)
{
    m_widgetOnTheSecondMonitor->setWindowTitle(msg);
}

void DisplayManager::setRuntimeLabel(const QString &msg)
{
    if(m_pmDisk)
    {
        m_pmDisk->setRuntimeLabel(msg);
    }
    {
        m_liveSceneView->setRuntimeLabel(msg);
    }
}

void DisplayManager::setCurrentTime(const QString &msg)
{
    if(m_pmDisk)
    {
        m_pmDisk->setCurrentTime(msg);
    }
    if(m_liveSceneView)
    {
        m_liveSceneView->setCurrentTime(msg);
    }
}

void DisplayManager::setDevice(const QString &msg)
{
    if(m_liveSceneView)
    {
        m_liveSceneView->setDevice(msg);
    }
}

void DisplayManager::setLabelLiveColor(const QString &msg)
{
    if(m_liveSceneView)
    {
        m_liveSceneView->setLabelLiveColor(msg);
    }
}

void DisplayManager::setBorderForRecording(const QString &styleSheet)
{
    m_liveSceneView->setBorderForRecording(styleSheet);
}

void DisplayManager::setRecordingEnabled(bool isEnabled)
{
    m_liveSceneView->setEnableRecording(isEnabled);
}

void DisplayManager::setRecordingChecked(bool isChecked)
{
    m_liveSceneView->setCheckRecording(isChecked);
}

void DisplayManager::pushButtonRecord_clicked(bool isChecked)
{
    m_liveSceneView->pushButtonRecord_clicked(isChecked);
}

void DisplayManager::setSpeedVisible(bool isVisible)
{
    m_liveSceneView->setSpeedVisible(isVisible);
}

void DisplayManager::setSpeed(const QString &speed)
{
    m_liveSceneView->setSpeed(speed);
}

DisplayManager::DisplayManager(QObject *parent) : QObject(parent)
{
    m_eventFileWatcher = std::make_unique< QFileSystemWatcher>();
    m_eventFileWatcher->addPath(m_eventFileName);

    connect(m_eventFileWatcher.get(), &QFileSystemWatcher::fileChanged, this, &DisplayManager::monitorEvent);

    m_diplaySettingsMonitor = std::make_unique<QProcess>();

    m_diplaySettingsMonitor->setArguments(m_programArguments);
    m_diplaySettingsMonitor->setProgram(m_programName);
    if(!userSettings::Instance().getDisableExternalMonitor()){
        m_diplaySettingsMonitor->start();
    }

    m_liveSceneView = std::make_unique<LiveSceneView>();
    m_pmDisk = std::make_unique<FormDisk>();
    m_pmLogo = std::make_unique<FormPmLogo>();

    m_widgetContainer["logo"] = m_pmLogo.get();
    m_widgetContainer["disk"] = m_pmDisk.get();
    m_widgetContainer["liveData"] = m_liveSceneView.get();

    connect(this, &DisplayManager::nonPrimaryMonitorIsPresent, this, &DisplayManager::showHideSecondMonitor);

    initSecondMonitor("logo");

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
