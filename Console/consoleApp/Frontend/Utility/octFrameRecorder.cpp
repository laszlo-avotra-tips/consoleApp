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
    if(m_count % 64 == 0){
        LOG2(m_count, frame)
    }
    ++m_count;
}

OctFrameRecorder::OctFrameRecorder(QObject *parent) : QObject(parent)
{

}
