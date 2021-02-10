#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QString>
#include <memory>
#include <map>

class QFileSystemWatcher;
class LiveSceneView;
class FormPmLogo;
class QGraphicsView;
class liveScene;

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
    void setScene(liveScene* scene);
    void initWidgetForTheSecondMonitor(QString name);
    void initSecondMonitor(QString name);

signals:
    void nonPrimaryMonitorIsPresent(bool isPresent);

public slots:
    void monitorEvent(const QString& fileName);
    void showHideSecondMonitor(bool isNonPrimaryMonitorPresent);

private slots:
    void programFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    static DisplayManager* m_instance;
    std::unique_ptr<QProcess> m_diplaySettingsMonitor{nullptr};
    std::unique_ptr<QFileSystemWatcher> m_eventFileWatcher{nullptr};
    std::unique_ptr<LiveSceneView> m_liveSceneView{nullptr};
    std::unique_ptr<FormPmLogo> m_pmLogo{nullptr};
    QWidget* m_widgetOnTheSecondMonitor{nullptr};

    const QString m_programName{R"(MonWMIServer.exe)"};
    const QString m_eventFileName{R"(C:\Avinger_System\MonitorEvent.txt)"};
    const QString m_logFileName{R"(C:\Avinger_System\MonitorEvent.log)"};
    const QStringList m_programArguments{"-w", "1280", "-h", "1024", "-e", m_eventFileName, "-l", m_logFileName};

    bool m_isNonPrimaryMonitorPresent{false};

    std::map<QString, QWidget*> m_widgetContainer;

private:
    DisplayManager(QObject *parent = nullptr);
    void parseEventFile(const QString& eventFileName);
    void parse(const QString& line);


};

#endif // DISPLAYMANAGER_H
