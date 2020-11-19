#include "octFrameRecorder.h"

OctFrameRecorder* OctFrameRecorder::m_instance{nullptr};

OctFrameRecorder *OctFrameRecorder::instance()
{
    if(!m_instance){
        m_instance = new OctFrameRecorder();
    }
    return m_instance;
}

OctFrameRecorder::OctFrameRecorder(QObject *parent) : QObject(parent)
{

}
