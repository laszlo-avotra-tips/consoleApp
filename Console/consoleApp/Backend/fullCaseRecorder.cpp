#include "fullCaseRecorder.h"
#include "logger.h"

#include <QProcess>
#include <QFile>
#include <QStringList>
#include <QThread>


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

void FullCaseRecorder::setFullCaseDir(const QString &fullCaseDir)
{
    if(m_caseId.isEmpty()){
        m_caseId = fullCaseDir;
        m_theVideoRecorderProcess = new QProcess();

        const QString& outputDirectory = fullCaseDir;
        QStringList arguments {m_configFileName, outputDirectory, m_commandFileName};
        m_theVideoRecorderProcess->setArguments(arguments);
        m_theVideoRecorderProcess->setProgram(m_programName);
        m_theVideoRecorderProcess->start();
        LOG3(m_configFileName, outputDirectory, m_commandFileName)
        QThread::sleep(1);
        startRecording();
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
    LOG2(m_commandFileName,m_stopRecording.toLatin1())
}

void FullCaseRecorder::closeRecorder()
{
    stopRecording();
    QThread::msleep(100);
    m_theVideoRecorderProcess->close();
}
