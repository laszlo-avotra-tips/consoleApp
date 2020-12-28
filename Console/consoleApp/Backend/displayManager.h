#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>
#include <QString>

class QProcess;
//MonWMIServer.exe -w 1280 -h 1024 -e C:\work\MonEvent.txt -l C:\Work\MonWMIServer.log

class DisplayManager : public QObject
{
    Q_OBJECT
public:
    static DisplayManager* instance();

private:
    static DisplayManager* m_instance;
    QString m_cmdLine{R"(MonWMIServer.exe -w 1280 -h 1024 -e C:\Avinger_System\MonitorEvent.txt -l C:\Avinger_System\MonitorEvent.log)"};
    QProcess* m_diplaySettingsMonitor{nullptr};
    QString m_programName{R"(MonWMIServer.exe)"};
    QString m_programArguments{R"(-w 1280 -h 1024 -e C:\Avinger_System\MonitorEvent.txt -l C:\Avinger_System\MonitorEvent.log)"};

    DisplayManager(QObject *parent = nullptr);

signals:

};

#endif // DISPLAYMANAGER_H
