#include "displayManager.h"
#include <QProcess>

DisplayManager* DisplayManager::m_instance{nullptr};

DisplayManager *DisplayManager::instance()
{
    if(!m_instance){
        m_instance = new DisplayManager();
    }
    return m_instance;
}

DisplayManager::DisplayManager(QObject *parent) : QObject(parent)
{
    m_diplaySettingsMonitor = new QProcess();

    m_diplaySettingsMonitor->setArguments(m_programArguments);
    m_diplaySettingsMonitor->setProgram(m_programName);
    m_diplaySettingsMonitor->start();
}
