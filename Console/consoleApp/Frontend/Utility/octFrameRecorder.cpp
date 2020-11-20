#include "octFrameRecorder.h"
#include <logger.h>
#include <string>


OctFrameRecorder* OctFrameRecorder::m_instance{nullptr};

OctFrameRecorder *OctFrameRecorder::instance()
{
    if(!m_instance){
        m_instance = new OctFrameRecorder();
    }
    return m_instance;
}

void OctFrameRecorder::recordData(uint8_t *dispData)
{
    if(dispData && m_recorderIsOn){
        if(!m_playlistFileName.isEmpty() && !m_outDirPath.isEmpty() && m_screenCapture){
            m_screenCapture->encodeFrame(dispData);
        }
    }
}

OctFrameRecorder::OctFrameRecorder(QObject *parent) : QObject(parent)
{
    m_screenCapture = new CapUtils::ScreenCapture();
}

QString OctFrameRecorder::playlistFileName() const
{
    return m_playlistFileName;
}

void OctFrameRecorder::setPlaylistFileName(const QString &playlistFileName)
{
    m_playlistFileName = playlistFileName;
    LOG1(m_playlistFileName)
}

bool OctFrameRecorder::start()
{
    bool success{false};
    if(!m_outDirPath.isEmpty() && !m_playlistFileName.isEmpty() && m_screenCapture){
        const std::string directoryName {m_outDirPath.toStdString()};
        const std::string fileName {m_playlistFileName.toStdString()};
        LOG2(directoryName.c_str(),fileName.c_str())
        success = m_screenCapture->start(directoryName.c_str(), fileName.c_str());
        LOG1(success)
        if(success){
            QThread::msleep(500);
            setRecorderIsOn(true);
        }
    }
    return success;
}

bool OctFrameRecorder::stop()
{
    bool success{false};
    if(m_screenCapture){
        setRecorderIsOn(false);
        m_screenCapture->stop();
        success = true;
    }
    return success;
}

QString OctFrameRecorder::outDirPath() const
{
    return m_outDirPath;
}

void OctFrameRecorder::setOutDirPath(const QString &outDirPath)
{
    m_outDirPath = outDirPath;
    LOG1(m_outDirPath)
}

bool OctFrameRecorder::recorderIsOn() const
{
    return m_recorderIsOn;
}

void OctFrameRecorder::setRecorderIsOn(bool recorderIsOn)
{
    m_recorderIsOn = recorderIsOn;
}
