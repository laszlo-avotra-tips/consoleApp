#include "octFrameRecorder.h"
#include <logger.h>


OctFrameRecorder* OctFrameRecorder::m_instance{nullptr};

OctFrameRecorder *OctFrameRecorder::instance()
{
    if(!m_instance){
        m_instance = new OctFrameRecorder();
    }
    return m_instance;
}

void OctFrameRecorder::handleOctFrame(OCTFile::OctData_t *frame)
{
    if(frame && m_recorderIsOn){
//        if(m_count % 64 == 0){
//            LOG2(m_count, frame)
//        }
        ++m_count;
        if(!m_playlistFileName.isEmpty() && !m_outDirPath.isEmpty() && m_screenCapture){
            LOG2(m_count, frame->acqData)
//           m_screenCapture->encodeFrame(frame->acqData);
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
        const char* directory {m_outDirPath.toLatin1()};
        const char* fileName {m_playlistFileName.toLatin1()};
        LOG2(directory,fileName)
        m_screenCapture->start(directory, fileName);
        setRecorderIsOn(true);
        success = true;
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
