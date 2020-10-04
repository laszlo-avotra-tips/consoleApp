#include "fullCaseRecorder.h"
#include "logger.h"

#include <QProcess>
#include <QFile>
#include <QStringList>
#include <QThread>
#include <QTimer>


FullCaseRecorder* FullCaseRecorder::m_instance{nullptr};

FullCaseRecorder::~FullCaseRecorder()
{
    delete m_theVideoRecorderProcess;
    LOG1(m_theVideoRecorderProcess)
}

FullCaseRecorder *FullCaseRecorder::instance()
{
    if(!m_instance){
      m_instance = new FullCaseRecorder();
    }

    return m_instance;
}

FullCaseRecorder::FullCaseRecorder()
{
}

void FullCaseRecorder::startKeepAliveTimer()
{
    QTimer::singleShot(m_keepAliveTimerValue, this, &FullCaseRecorder::keepAliveAction);
}

void FullCaseRecorder::stopKeepAliveTimer()
{

}

void FullCaseRecorder::setFullCaseDir(const QString &fullCaseDir)
{
    if(m_caseId.isEmpty()){
        m_caseId = fullCaseDir;
        m_theVideoRecorderProcess = new QProcess();

        const QString& outputDirectory = fullCaseDir;
        QStringList arguments {m_configFileName, outputDirectory, m_commandFileName, m_keepAliveFrequencyCmd};
        m_theVideoRecorderProcess->setArguments(arguments);
        m_theVideoRecorderProcess->setProgram(m_programName);
        m_theVideoRecorderProcess->start();
        m_isRecording = true;
//        startKeepAliveTimer();
        LOG3(m_configFileName, outputDirectory, m_commandFileName) LOG1( m_keepAliveFrequencyCmd)
    }
}

void FullCaseRecorder::startRecording()
{
    QFile cmd(m_commandFileName);
    cmd.open(QIODevice::WriteOnly | QIODevice::Text);
    cmd.write(m_startRecording.toLatin1(), m_startRecording.size());
    cmd.close();
    LOG2(m_commandFileName,m_startRecording.toLatin1())
}

void FullCaseRecorder::stopRecording()
{
    QFile cmd(m_commandFileName);
    cmd.open(QIODevice::WriteOnly | QIODevice::Text);
    cmd.write(m_stopRecording.toLatin1(), m_stopRecording.size());
    cmd.close();
    m_isRecording = false;
    LOG2(m_commandFileName,m_stopRecording.toLatin1())
    stopKeepAliveTimer();
}

void FullCaseRecorder::closeRecorder()
{
    LOG1(m_theVideoRecorderProcess)
    stopRecording();
}

void FullCaseRecorder::keepAliveAction()
{
    QTimer::singleShot(m_keepAliveTimerValue, this, &FullCaseRecorder::keepAliveAction);
    if(m_isRecording){
        startRecording();
    }
}
