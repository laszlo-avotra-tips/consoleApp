
#include "theglobals.h"
#include "daqSettings.h"
#include "playbackmanager.h"
#include <QMutexLocker>
#include "logger.h"

#include <memory>

TheGlobals* TheGlobals::m_instance(nullptr);

TheGlobals *TheGlobals::instance()
{
    if(!m_instance){
        m_instance = new TheGlobals();
    }
    return m_instance;
}

TheGlobals::TheGlobals()
{
    allocateOctData();
}

void TheGlobals::allocateOctData()
{
    DaqSettings &settings = DaqSettings::Instance();

    const size_t rawDataSize{settings.getRecordLength()}; //4096
    const size_t fftDataSize{FFTDataSize};
    const size_t dispDataSize{SectorHeight_px * SectorWidth_px};

    LOG3(rawDataSize, fftDataSize, dispDataSize); //8192, 4096, 1024, 1982464

    for(int i = 0; i < 3; ++i){
//  for(int i = 0; i < FRAME_BUFFER_SIZE; ++i){

        OCTFile::OctData_t oct;

        oct.advancedViewIfftData   = new quint16 [rawDataSize];
        oct.advancedViewFftData   = new quint16 [fftDataSize];
        oct.dispData  = new uchar [dispDataSize];
        oct.videoData = new uchar [dispDataSize];

        m_octData.push_back(oct);
    }

}

void TheGlobals::pushImageRenderingQueue(int index)
{
    QMutexLocker guard(&m_imageRenderingMutex);
    m_imageRenderingQueue.push(index);
}

void TheGlobals::popImageRenderingQueue()
{
    QMutexLocker guard(&m_imageRenderingMutex);
    m_imageRenderingQueue.pop();
}

bool TheGlobals::isImageRenderingQueue() const
{
    return !m_imageRenderingQueue.empty();
}

int TheGlobals::frontImageRenderingQueue() const
{
    if(isImageRenderingQueue()){
        return m_imageRenderingQueue.front();
    }
    return -1;
}

void TheGlobals::freeOctData()
{
    for(auto it = m_octData.begin(); it != m_octData.end(); ++it){
        delete [] it->advancedViewIfftData;
        delete [] it->advancedViewFftData;
        delete [] it->dispData;
        delete [] it->videoData;
    }
    m_octData.clear();
}

OCTFile::OctData_t *TheGlobals::getOctData(int index)
{
    OCTFile::OctData_t * retVal(nullptr);

    size_t frameDataIndex{ size_t(index) % m_octData.size()};
    auto& val = m_octData[frameDataIndex];
    val.frameCount = index;
    retVal = &val;
    return retVal;
}
