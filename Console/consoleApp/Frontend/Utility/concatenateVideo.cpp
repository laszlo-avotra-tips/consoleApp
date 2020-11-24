#include "concatenateVideo.h"
#include <logger.h>

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
    LOG1(m_outputPath);
    LOG1(m_inputPlaylistFile);
    LOG1(m_outputLoopFile);
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
