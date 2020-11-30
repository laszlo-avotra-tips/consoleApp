#include "octFrameRecorder.h"
#include <logger.h>
#include <string>
#include "Utility/userSettings.h"
#include "Utility/concatenateVideo.h"


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
        if(!playlistFileName().isEmpty() && !outDirPath().isEmpty() && m_screenCapture){
            m_screenCapture->encodeFrame(dispData);
        }
    }
}

OctFrameRecorder::OctFrameRecorder(QObject *parent) : QObject(parent)
{
    m_screenCapture = new CapUtils::ScreenCapture();
    m_concatenateVideo = ConcatenateVideo::instance();
}

void OctFrameRecorder::updateOutputFileName(int loopNumber)
{
    // tag the images as "LOOP1, LOOP2, ..."
    setPlaylistThumbnail( QString("LOOP%1").arg(loopNumber));
    setPlaylistFileName( playlistThumbnail() + QString( ".m3u8" ));
    caseInfo &info = caseInfo::Instance();
    QString dirName = info.getStorageDir() + "/clips";
    QDir thisDir(dirName);
    QString subDirName = playlistThumbnail();
    thisDir.mkdir(subDirName);

    setOutDirPath( QString("%1/%2/").arg(dirName).arg(subDirName)); // Set up the absolute path based on the session data.

    LOG3(dirName, subDirName, playlistFileName())
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
    m_concatenateVideo->setOutputLoopFile(playlistThumbnail + QString(".mp4"));
}

QString OctFrameRecorder::playlistFileName() const
{
    return m_playlistFileName;
}

void OctFrameRecorder::setPlaylistFileName(const QString &playlistFileName)
{
    m_playlistFileName = playlistFileName;
    m_concatenateVideo->setInputPlaylistFile(playlistFileName);
    LOG1(playlistFileName)
}

bool OctFrameRecorder::start()
{
    bool success{false};
    if(!outDirPath().isEmpty() && !playlistFileName().isEmpty() && m_screenCapture){
        const std::string directoryName {outDirPath().toStdString()};
        const std::string fileName {playlistFileName().toStdString()};
        LOG2(directoryName.c_str(),fileName.c_str())
        success = m_screenCapture->start(directoryName.c_str(), fileName.c_str());
        LOG1(success)
        if(success){
//            QThread::msleep(500);
            setRecorderIsOn(true);
        }
    }
    return success;
}

bool OctFrameRecorder::stop()
{
    bool success{false};
    if(m_screenCapture && recorderIsOn()){
        setRecorderIsOn(false);
        m_screenCapture->stop();
        success = true;
    }
    LOG1(success)
    return success;
}

QString OctFrameRecorder::outDirPath() const
{
    return m_outDirPath;
}

void OctFrameRecorder::setOutDirPath(const QString &outDirPath)
{
    m_concatenateVideo->setOutputPath(outDirPath);
    m_outDirPath = outDirPath;
    LOG1(outDirPath)
}

bool OctFrameRecorder::recorderIsOn() const
{
    return m_recorderIsOn;
}

void OctFrameRecorder::setRecorderIsOn(bool recorderIsOn)
{
    if(m_recorderIsOn && !recorderIsOn){
        m_concatenateVideo->execute();
    }
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
