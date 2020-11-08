#include "saveFrame.h"
#include <QFile>
#include "logger.h"


SaveFrame* SaveFrame::m_instance = nullptr;

SaveFrame* SaveFrame::instance()
{
    if(!m_instance) {
        m_instance = new SaveFrame();
    }
    return m_instance;
}

void SaveFrame::saveBuffer(const uint8_t *buffer, int count)
{
    if(m_isFile){
        const char* buff = reinterpret_cast<const char*>(buffer);
        if((count > m_countBegin) && count <= m_countEnd)
        {
            auto bw = m_file->write(buff, 1024*1024 );
            if(bw != -1){
                m_bytesWritten += bw;
            } else {
                LOG2("ERROR",bw)
            }
        }
    }

}

quint64 SaveFrame::bytesWritten() const
{
    return m_bytesWritten;
}

SaveFrame::SaveFrame()
{
    m_file = new QFile(m_fn);
    m_isFile = m_file->open(QIODevice::Append);
    LOG1(m_isFile);
}
