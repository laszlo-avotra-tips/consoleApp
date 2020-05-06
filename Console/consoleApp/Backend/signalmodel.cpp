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

//    for(int i = 0; i < 3; ++i){
    for(int i = 0; i < FRAME_BUFFER_SIZE; ++i){

        OCTFile::OctData_t oct;

        oct.advancedViewIfftData  = new uint8_t [fftDataSize];
        oct.advancedViewFftData   = new uint8_t [fftDataSize];
        oct.dispData  = new uint8_t [dispDataSize];
        oct.videoData = new uint8_t [dispDataSize];
        oct.acqData   = new uint8_t [MAX_ACQ_IMAGE_SIZE];

        m_octData.push_back(oct);
    }

}

const cl_int* SignalModel::getSectorHeight_px() const
{
    return &m_sectorHeight_px;
}

const cl_int* SignalModel::getSectorWidth_px() const
{
    return &m_sectorWidth_px;
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

const cl_int *SignalModel::getImagingDepth_S() const
{
    return &m_imagingDepth_S;
}

void SignalModel::setImagingDepth_S(const cl_int &imagingDepth_S)
{
    m_imagingDepth_S = imagingDepth_S;
}

const cl_float *SignalModel::getFractionOfCanvas() const
{
    return &m_fractionOfCanvas;
}

void SignalModel::setFractionOfCanvas(const cl_float &fractionOfCanvas)
{
    m_fractionOfCanvas = fractionOfCanvas;
}

const cl_int *SignalModel::getIsDistalToProximalView() const
{
    return &m_isDistalToProximalView;
}

void SignalModel::setIsDistalToProximalView(const cl_int &isDistalToProximalView)
{
    m_isDistalToProximalView = isDistalToProximalView;
}

const cl_float *SignalModel::getDisplayAngle() const
{
    return &m_displayAngle;
}

void SignalModel::setDisplayAngle(const cl_float &displayAngle)
{
    m_displayAngle = displayAngle;
}

const cl_int *SignalModel::getALineLengthNormal_px() const
{
    return &m_aLineLengthNormal_px;
}

void SignalModel::setALineLengthNormal_px(const cl_int &aLineLengthNormal_px)
{
    m_aLineLengthNormal_px = aLineLengthNormal_px;
}

const cl_float *SignalModel::getImagingDehthNormal_mm() const
{
    return &m_imagingDepthNormal_mm;
}

void SignalModel::setImagingDepthNormal_mm(const cl_float &imagingDepthNormal_mm)
{
    m_imagingDepthNormal_mm = imagingDepthNormal_mm;
}

const cl_float *SignalModel::getInternalImagingMask_px() const
{
    return &m_internalImagingMask_px;
}

void SignalModel::setInternalImagingMask_px(const cl_float &internalImagingMask_px)
{
    m_internalImagingMask_px = internalImagingMask_px;
}

const cl_float *SignalModel::getCatheterRadius_um() const
{
    return &m_catheterRadius_um;
}

void SignalModel::setCatheterRadius_um(const cl_float &catheterRadius_um)
{
    m_catheterRadius_um = catheterRadius_um;
}

const cl_mem *SignalModel::getBeAndCeImageBuffer() const
{
    return &m_bAndCimageBuffer;
}

void SignalModel::setPostBandcImageBuffer(const cl_mem &value)
{
    m_bAndCimageBuffer = value;
}

const cl_int* SignalModel::whiteLevel() const
{
    return &m_whiteLevel;
}

void SignalModel::setWhiteLevel(int whiteLevel)
{
    m_whiteLevel = whiteLevel;
}

const cl_int *SignalModel::blackLevel() const
{
    return &m_blackLevel;
}

void SignalModel::setBlackLevel(int blackLevel)
{
    m_blackLevel = blackLevel;
}

const cl_mem* SignalModel::fftImageBuffer() const
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

const cl_uint* SignalModel::getInputLength() const
{
    return &m_inputLength;
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

const cl_float* SignalModel::currFrameWeight_percent() const
{
    return &m_currFrameWeight_percent;
}

void SignalModel::setCurrFrameWeight_percent(int currFrameWeight_percent)
{
    m_currFrameWeight_percent = 0.01f * currFrameWeight_percent;
    m_prevFrameWeight_percent = 0.01f * (100 - currFrameWeight_percent);
}

const cl_int* SignalModel::isInvertOctColors() const
{
    return &m_isInvertOctColors;
}

void SignalModel::setIsInvertColors(bool isInvertOctColors)
{
    m_isInvertOctColors = isInvertOctColors;
}

const cl_int* SignalModel::isAveragingNoiseReduction() const
{
    return &m_isAveragingNoiseReduction;
}

void SignalModel::setIsAveragingNoiseReduction(bool isAveragingNoiseReduction)
{
    m_isAveragingNoiseReduction = isAveragingNoiseReduction;
}

void SignalModel::pushImageRenderingQueue(const OctData& od)
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
