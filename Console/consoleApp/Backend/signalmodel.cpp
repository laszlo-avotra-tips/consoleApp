#include "signalmodel.h"
#include "daqSettings.h"
#include "logger.h"


SignalModel* SignalModel::m_instance{nullptr};

SignalModel::SignalModel()
{
    allocateOctData();
}

void SignalModel::allocateOctData()
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

cl_mem SignalModel::getWarpVideoBuffer()
{
    return m_warpVideoBuffer;
}

void SignalModel::setWarpVideoBuffer(const cl_mem &warpVideoBuffer)
{
    m_warpVideoBuffer = warpVideoBuffer;
}

cl_mem SignalModel::getWarpImageBuffer()
{
    return m_warpImageBuffer;
}

void SignalModel::setWarpImageBuffer(const cl_mem &warpImageBuffer)
{
    m_warpImageBuffer = warpImageBuffer;
}

cl_int* SignalModel::getImagingDepth()
{
    return &m_imagingDepth;
}

void SignalModel::setImagingDepth(const cl_int &imagingDepth)
{
    m_imagingDepth = imagingDepth;
}

cl_float* SignalModel::getFractionOfCanvas()
{
    return &m_fractionOfCanvas;
}

void SignalModel::setFractionOfCanvas(const cl_float &fractionOfCanvas)
{
    m_fractionOfCanvas = fractionOfCanvas;
}

cl_int* SignalModel::getReverseDirection()
{
    return &m_reverseDirection;
}

void SignalModel::setReverseDirection(const cl_int &reverseDirection)
{
    m_reverseDirection = reverseDirection;
}

cl_float* SignalModel::getDisplayAngle()
{
    return &m_displayAngle;
}

void SignalModel::setDisplayAngle(const cl_float &displayAngle)
{
    m_displayAngle = displayAngle;
}

cl_int* SignalModel::getStandardDehthS()
{
    return &m_standardDehthS;
}

void SignalModel::setStandardDehthS(const cl_int &standardDehthS)
{
    m_standardDehthS = standardDehthS;
}

cl_float* SignalModel::getStandardDehthMm()
{
    return &m_standardDepthMm;
}

void SignalModel::setStandardDehthMm(const cl_float &standardDehthMm)
{
    m_standardDepthMm = standardDehthMm;
}

cl_float *SignalModel::getInternalImagingMaskPx()
{
    return &m_internalImagingMaskPx;
}

void SignalModel::setInternalImagingMaskPx(const cl_float &internalImagingMaskPx)
{
    m_internalImagingMaskPx = internalImagingMaskPx;
}

cl_float *SignalModel::getCatheterRadiusUm()
{
    return &m_catheterRadiusUm;
}

void SignalModel::setCatheterRadiusUm(const cl_float &catheterRadiusUm)
{
    m_catheterRadiusUm = catheterRadiusUm;
}

cl_mem* SignalModel::getBeAndCeImageBuffer()
{
    return &m_bAndCimageBuffer;
}

void SignalModel::setPostBandcImageBuffer(const cl_mem &value)
{
    m_bAndCimageBuffer = value;
}

cl_float* SignalModel::whiteLevel()
{
    return &m_whiteLevel;
}

void SignalModel::setAveraging(bool isOn)
{
    setAverageVal(isOn);
}

void SignalModel::setInvertColors(bool isInverted)
{
    setIsInvertColors(isInverted);
}

void SignalModel::setFrameAverageWeights(int prev, int curr)
{
    setPrevFrameWeight_percent(prev * 0.01f);
    setCurrFrameWeight_percent(curr * 0.01f);
}

void SignalModel::setWhiteLevel(int whiteLevel)
{
    m_whiteLevel = whiteLevel;
}

cl_float *SignalModel::blackLevel()
{
    return &m_blackLevel;
}

void SignalModel::setBlackLevel(int blackLevel)
{
    m_blackLevel = blackLevel;
}

cl_mem* SignalModel::fftImageBuffer()
{
    return &m_fftImageBuffer;
}

void SignalModel::setFftImageBuffer(const cl_mem &fftImageBuffer)
{
    m_fftImageBuffer = fftImageBuffer;
}

SignalModel *SignalModel::instance()
{
    if(!m_instance){
        m_instance = new SignalModel();
    }
    return m_instance;
}

const cl_uint* SignalModel::iputLength() const
{
    return &m_iputLength;
}

const cl_float* SignalModel::scaleFactor() const
{
    return &m_scaleFactor;
}

cl_uint SignalModel::linesPerRevolution() const
{
    return m_linesPerRevolution;
}

void SignalModel::setLinesPerRevolution(const cl_uint &linesPerRevolution)
{
    m_linesPerRevolution = linesPerRevolution;
}

const cl_uint* SignalModel::dcNoiseLevel() const
{
    return &m_dcNoiseLevel;
}

const cl_float* SignalModel::prevFrameWeight_percent() const
{
    return &m_prevFrameWeight_percent;
}

void SignalModel::setPrevFrameWeight_percent(const cl_float &prevFrameWeight_percent)
{
    m_prevFrameWeight_percent = prevFrameWeight_percent;
}

const cl_float* SignalModel::currFrameWeight_percent() const
{
    return &m_currFrameWeight_percent;
}

void SignalModel::setCurrFrameWeight_percent(const cl_float &currFrameWeight_percent)
{
    m_currFrameWeight_percent = currFrameWeight_percent;
}

const cl_int* SignalModel::isInvertColors() const
{
    return &m_isInvertColors;
}

void SignalModel::setIsInvertColors(const cl_int &isInvertColors)
{
    m_isInvertColors = isInvertColors;
}

const cl_int* SignalModel::averageVal() const
{
    return &m_averageVal;
}

void SignalModel::setAverageVal(const cl_int &averageVal)
{
    m_averageVal = averageVal;
}

void SignalModel::pushImageRenderingQueue(OctData od)
{
    QMutexLocker guard(&m_imageRenderingMutex);
    m_imageRenderingQueue.push(od);
}

void SignalModel::popImageRenderingQueue()
{
    QMutexLocker guard(&m_imageRenderingMutex);
    m_imageRenderingQueue.pop();
}

bool SignalModel::isImageRenderingQueueGTE(size_t length) const
{
    return m_imageRenderingQueue.size() >= length;
}

std::pair<bool, OctData> SignalModel::frontImageRenderingQueue()
{
    std::pair<bool, OctData> retVal{false, OctData()};
    if(isImageRenderingQueueGTE(2)){
        retVal.second = m_imageRenderingQueue.front();
        retVal.first = true;
    }
    return retVal;
}

void SignalModel::freeOctData()
{
    for(auto it = m_octData.begin(); it != m_octData.end(); ++it){
        delete [] it->advancedViewIfftData;
        delete [] it->advancedViewFftData;
        delete [] it->dispData;
        delete [] it->videoData;
    }
    m_octData.clear();
}

OCTFile::OctData_t *SignalModel::getOctData(int index)
{
    OCTFile::OctData_t * retVal(nullptr);

    size_t frameDataIndex{ size_t(index) % m_octData.size()};
    auto& val = m_octData[frameDataIndex];
    val.frameCount = index;
    retVal = &val;
    return retVal;
}
