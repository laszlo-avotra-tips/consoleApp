#include "octFrameRecorder.h"
#include <logger.h>
#include <string>
#include "Utility/userSettings.h"


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

void OctFrameRecorder::updateOutputFileName(int loopNumber)
{
    // tag the images as "LOOP 1, LOOP 2, ..."
    m_playlistThumbnail = QString("LOOP %1").arg(loopNumber);
    m_playlistFileName = m_playlistThumbnail + QString( ".m3u8" );
//    LOG1(m_playlistFileName)
    caseInfo &info = caseInfo::Instance();
    QDir thisDir(info.getStorageDir());
    thisDir.mkdir(m_playlistFileName);

//    m_outDirPath = info.getStorageDir() + "/clips/"; // Set up the absolute path based on the session data.
    m_outDirPath = QString("%1/clips/%2/").arg(info.getStorageDir()).arg(m_playlistFileName); // Set up the absolute path based on the session data.
}

void OctFrameRecorder::updateClipList(int loopNumber)
{
    caseInfo       &info        = caseInfo::Instance();
    deviceSettings &dev = deviceSettings::Instance();

    const QString ClipName = QString("clip-%1").arg(loopNumber);
    QString clipFilename = info.getClipsDir() + "/" + ClipName;

}

QString OctFrameRecorder::playlistThumbnail() const
{
    return m_playlistThumbnail;
}

void OctFrameRecorder::setPlaylistThumbnail(const QString &playlistThumbnail)
{
    m_playlistThumbnail = playlistThumbnail;
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

void OctFrameRecorder::onRecordSector(bool isRecording)
{
    if(isRecording){
        m_currentLoopNumber++;
        updateOutputFileName(m_currentLoopNumber);
        updateClipList(m_currentLoopNumber);
        OctFrameRecorder::instance()->start();
    } else {
        OctFrameRecorder::instance()->stop();
    }
}
