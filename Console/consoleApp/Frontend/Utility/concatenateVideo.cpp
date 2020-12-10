#include "concatenateVideo.h"
#include <logger.h>
#include <QProcess>
#include <map>

ConcatenateVideo* ConcatenateVideo::m_instance = nullptr;

ConcatenateVideo *ConcatenateVideo::instance()
{
    if(!m_instance){
        m_instance = new ConcatenateVideo();
    }
    return m_instance;
}

ConcatenateVideo::ConcatenateVideo(QObject *parent) : QObject(parent)
{
    m_concatenateProcess = std::make_unique<QProcess>();

    connect(    m_concatenateProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ConcatenateVideo::programFinished);
}

QString ConcatenateVideo::outputLoopFile() const
{
    return m_outputLoopFile;
}

void ConcatenateVideo::setOutputLoopFile(const QString &outputLoopFile)
{
    m_outputLoopFile = outputLoopFile;
}

void ConcatenateVideo::execute()
{
    //concat_streams -p <Output path> -i <Input playlist file> -o <Output loop file>
    QStringList arguments { "-p", m_outputPath, "-i", m_inputPlaylistFile, "-o", m_outputLoopFile};

    LOG1(m_outputPath);
    LOG1(m_inputPlaylistFile);
    LOG1(m_outputLoopFile);

    m_concatenateProcess->setArguments(arguments);
    m_concatenateProcess->setProgram(m_concatenateProgramName);
    m_concatenateProcess->start();
}

void ConcatenateVideo::programFinished(int exitCode, QProcess::ExitStatus exitStatus)
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

    emit executionDone(exitCode, message);

    LOG2(exitCode, message)
}

QString ConcatenateVideo::inputPlaylistFile() const
{
    return m_inputPlaylistFile;
}

void ConcatenateVideo::setInputPlaylistFile(const QString &inputPlaylistFile)
{
    m_inputPlaylistFile = inputPlaylistFile;
}

QString ConcatenateVideo::outputPath() const
{
    return m_outputPath;
}

void ConcatenateVideo::setOutputPath(const QString &outputPath)
{
    m_outputPath = outputPath;
}
