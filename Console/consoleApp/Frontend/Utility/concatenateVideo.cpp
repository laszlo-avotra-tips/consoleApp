#include "concatenateVideo.h"

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
