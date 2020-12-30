#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTextStream>


class QProcess;
class QFileSystemWatcher;
class FormSecondMonitor;

//MonWMIServer.exe -w 1280 -h 1024 -e C:\work\MonEvent.txt -l C:\Work\MonWMIServer.log

class DisplayManager : public QObject
{
    Q_OBJECT
public:
    static DisplayManager* instance();
    void killDisplayMonitor();

    QString eventFileName() const;

    bool isNonPrimaryMonitorPresent() const;
    void setIsNonPrimaryMonitorPresent(bool isNonPrimaryMonitorPresent);

signals:
    void nonPrimaryMonitorIsPresent(bool isPresent);

public slots:
    void monitorEvent(const QString& fileName);
    void showSecondMonitor(bool isNonPrimaryMonitorPresent);

private:
    static DisplayManager* m_instance;
    QString m_cmdLine{R"(MonWMIServer.exe -w 1280 -h 1024 -e C:\Avinger_System\MonitorEvent.txt -l C:\Avinger_System\MonitorEvent.log)"};
    QProcess* m_diplaySettingsMonitor{nullptr};
    QFileSystemWatcher* m_eventFileWatcher{nullptr};
    const QString m_programName{R"(MonWMIServer.exe)"};
    const QString m_eventFileName{R"(C:\Avinger_System\MonitorEvent.txt)"};
    const QString m_logFileName{R"(C:\Avinger_System\MonitorEvent.log)"};
    const QStringList m_programArguments{"-w", "1280", "-h", "1024", "-e", m_eventFileName, "-l", m_logFileName};
    bool m_isNonPrimaryMonitorPresent{false};
    FormSecondMonitor* m_secondMonitor{nullptr};

private:
    DisplayManager(QObject *parent = nullptr);
    void parseEventFile(const QString& eventFileName);
    void parse(const QString& line);


};

#endif // DISPLAYMANAGER_H
