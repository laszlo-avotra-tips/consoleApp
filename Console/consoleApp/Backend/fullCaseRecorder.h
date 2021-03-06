#ifndef FULLCASERECORDER_H
#define FULLCASERECORDER_H

#include <QObject>
#include <QString>
#include <memory>

class QProcess;

class FullCaseRecorder : public QObject
{
  Q_OBJECT
public:
  ~FullCaseRecorder();
  static FullCaseRecorder* instance();

  void setFullCaseDir(const QString &fullCaseDir);
  void startRecording();
  void stopRecording();
  void closeRecorder();

public slots:
  void keepAliveAction();

private:
  FullCaseRecorder();
  void startTheVideoRecorderProcess();
  static  FullCaseRecorder* m_instance;
  void startKeepAliveTimer();
  void stopKeepAliveTimer();

  QProcess* m_theVideoRecorderProcess{nullptr};
  QString m_caseId;
  const QString m_programName{"VideoCapture.exe"};
  const QString m_configFileName{"C:/Avinger_System/config.json"};
  QString m_commandFileName{"C:/Avinger_System/VideoCommand.txt"};
  const QString m_startRecording{"StartRec"};
  const QString m_stopRecording{"StopRec"};
  const QString m_keepAliveFrequencyCmd{"30"}; // in seconds
  const int m_keepAliveTimerValue{5000}; //in milliseconds
  bool m_isRecording{false};
};

#endif // FULLCASERECORDER_H
