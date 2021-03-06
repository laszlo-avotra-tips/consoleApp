#include "octFrameRecorder.h"
#include <logger.h>
#include <string>
#include "Utility/userSettings.h"
#include "Utility/concatenateVideo.h"
#include "clipListModel.h"


OctFrameRecorder* OctFrameRecorder::m_instance{nullptr};

OctFrameRecorder *OctFrameRecorder::instance()
{
    if(!m_instance){
        m_instance = new OctFrameRecorder();
    }
    return m_instance;
}

void OctFrameRecorder::recordData(uint8_t *dispData, const char* catheterName, const char* cathalogName,
                                  const char* activePassive, const char* timeStamp, int width, int height)
{
    bool isOk {m_width == width && m_height == height};

    if(dispData && m_recorderIsOn && isOk)
    {
        if(!playlistFileName().isEmpty() && !clipListModel::Instance().getOutDirPath().isEmpty() && m_screenCapture)
        {
            m_screenCapture->encodeFrame
                    (
                        dispData,
                        catheterName,
                        cathalogName,
                        activePassive,
                        timeStamp
                        );
        }
    }
}

OctFrameRecorder::OctFrameRecorder(QObject *parent) : QObject(parent)
{
    m_screenCapture = new CapUtils::ScreenCapture();
    const QString logo{"C:/Avinger_System/logo_video.png"};
    LOG1(logo)
    m_screenCapture->setLogoPath(logo.toLatin1());
    m_concatenateVideo = ConcatenateVideo::instance();
}

void OctFrameRecorder::updateOutputFileName(int loopNumber)
{
    // tag the images as "LOOP1, LOOP2, ..."
    const QString playListThumbnail(QString("LOOP%1").arg(loopNumber));

    auto& clipList = clipListModel::Instance();

    clipList.setPlaylistThumbnail( playListThumbnail);
    setPlaylistFileName( clipListModel::Instance().getPlaylistThumbnail() + QString( ".m3u8" ));
    m_concatenateVideo->setOutputLoopFile(playListThumbnail + QString(".mp4"));

    caseInfo &info = caseInfo::Instance();
    QString dirName = info.getStorageDir() + "/clips";
    clipList.setThumbnailDir(dirName);
    QDir thisDir(dirName);
    QString subDirName = playListThumbnail;
    thisDir.mkdir(subDirName);

    const QString outDirPath(QString("%1/%2/").arg(dirName).arg(subDirName));
    clipListModel::Instance().setOutDirPath( outDirPath); // Set up the absolute path based on the session data.
    m_concatenateVideo->setOutputPath(outDirPath);

    LOG3(dirName, subDirName, playlistFileName())
}

void OctFrameRecorder::updateClipList(int loopNumber)
{
    LOG1(loopNumber)
    caseInfo       &info        = caseInfo::Instance();
    deviceSettings &dev = deviceSettings::Instance();

    const QString ClipName = QString("clip-%1").arg(loopNumber);
    QString clipFilename = info.getClipsDir() + "/" + ClipName;

}

QString OctFrameRecorder::timeStamp() const
{
    return m_timeStamp;
}

void OctFrameRecorder::setTimeStamp(const QString &timeStamp)
{
    m_timeStamp = timeStamp;
}

void OctFrameRecorder::setClipName(const QString &clipName)
{
    m_clipName = clipName;
}

QString OctFrameRecorder::clipName() const
{
    return m_clipName;
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
    if(!clipListModel::Instance().getOutDirPath().isEmpty() && !playlistFileName().isEmpty() && m_screenCapture && m_width > 0 && m_height >0){
        const std::string directoryName {clipListModel::Instance().getOutDirPath().toStdString()};
        const std::string fileName {playlistFileName().toStdString()};        
        LOG2(directoryName.c_str(),fileName.c_str());
        LOG1(clipName())
        success = m_screenCapture->start(directoryName.c_str(), fileName.c_str(), clipName().toLatin1(), m_width, m_height);
        LOG1(success)
        if(success){
            QThread::msleep(50);
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

bool OctFrameRecorder::recorderIsOn() const
{
    return m_recorderIsOn;
}

void OctFrameRecorder::setRecorderIsOn(bool recorderIsOn)
{
    if(m_recorderIsOn && !recorderIsOn){
        m_concatenateVideo->execute();
        //record is ready
        clipListModel& clipList = clipListModel::Instance();
        const auto& itemList = clipList.getAllItems();
        clipItem * item = itemList.last();
        if(item) {
            item->setIsReady(true);
        }
    }
    m_recorderIsOn = recorderIsOn;
}

void OctFrameRecorder::onRecordSector(bool isRecording)
{
    clipListModel& clipList = clipListModel::Instance();
    if(isRecording){
        const int currentLoopNumber = clipList.getCurrentLoopNumber() + 1;
        clipList.setCurrentLoopNumber(currentLoopNumber);
        updateOutputFileName(currentLoopNumber);
        updateClipList(currentLoopNumber);
    } else {
        OctFrameRecorder::instance()->stop();
    }
}
