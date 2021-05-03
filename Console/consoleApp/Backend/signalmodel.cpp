#include "signalmodel.h"
#include "logger.h"
#include "Utility/userSettings.h"
#include <QFile>


SignalModel* SignalModel::m_instance{nullptr};

SignalModel::SignalModel()
{
    allocateOctData();
    const auto& settings = userSettings::Instance();
    m_simulationFrameCount = settings.getStartFrame();
}

void SignalModel::allocateOctData()
{

    const size_t rawDataSize{4096}; //4096
    const size_t fftDataSize{FFTDataSize};
    const size_t dispDataSize{SectorHeight_px * SectorWidth_px};

    LOG3(rawDataSize, fftDataSize, dispDataSize); //8192, 4096, 1024, 1982464

    int frameBufferCount = userSettings::Instance().getNumberOfDaqBuffers() + int(userSettings::Instance().getIsSimulation());

    QMutexLocker guard(&m_imageRenderingMutex);
    for(int i = 0; i < frameBufferCount; ++i){

        OCTFile::OctData_t oct;

        oct.dispData  = new uint8_t [dispDataSize];
        oct.acqData   = new uint8_t [MAX_ACQ_IMAGE_SIZE];

        m_octData[i] = oct;
    }
}

void SignalModel::saveOct(const OctData &od)
{
    QString dir = userSettings::Instance().getSimDir();

    QString fn = m_simFnBase + dir + QString("/frame") + QString::number(od.frameCount) + QString(".dat");
    QFile file(fn);

    if(file.open(QFile::WriteOnly)){
        auto len = file.write(reinterpret_cast<const char*>(od.acqData), od.bufferLength * 1024);
//        LOG3(fn, od.bufferLength, len);
        file.close();
    }
}

bool SignalModel::retrieveOct(OctData &od)
{
    bool success = false;
    QString dir = userSettings::Instance().getSimDir();
    QString fn = m_simFnBase + dir + QString("/frame") + QString::number(od.frameCount) + QString(".dat");
    QFile file(fn);

    if(file.open(QFile::ReadOnly)){
        auto len = file.read(reinterpret_cast<char*>(od.acqData), MAX_ACQ_IMAGE_SIZE);
        od.bufferLength = len / 1024;
//        LOG3(fn, od.acqData, len);
        file.close();
        success = len > 0;
    } else {
        LOG1(fn)
    }

    return success;
}

const cl_float* SignalModel::getCatheterRadius_um() const
{
    return &m_catheterRadius_um;
}

void SignalModel::setCatheterRadius_um(const cl_float &catheterRadius_um)
{
    m_catheterRadius_um = catheterRadius_um;
}

const cl_float* SignalModel::getStandardDepth_mm() const
{
    return &m_standardDepth_mm;
}

void SignalModel::setStandardDepth_mm(const cl_float &standardDepth_mm)
{
    m_standardDepth_mm = standardDepth_mm;
}

const cl_int* SignalModel::getSectorHeight_px() const
{
    return &m_sectorHeight_px;
}

void SignalModel::setAdvacedViewSourceFrameNumber(int frameNumber)
{
    m_dvacedViewSourceFrameNumber = frameNumber;
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

const cl_int *SignalModel::getALineLength_px() const
{
    return &m_aLineLength_px;
}

void SignalModel::setALineLength_px(const cl_int &aLineLength_px)
{
    m_aLineLength_px = aLineLength_px;
}

const cl_float *SignalModel::getInternalImagingMask_px() const
{
    return &m_internalImagingMask_px;
}

void SignalModel::setInternalImagingMask_px(const cl_float &internalImagingMask_px)
{
    m_internalImagingMask_px = internalImagingMask_px;
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

void SignalModel::pushImageRenderingQueue(OctData &od)
{
    auto data = handleSimulationSettings(od);
    QMutexLocker guard(&m_imageRenderingMutex);
    m_imageRenderingQueue.push(data);
}

bool SignalModel::isImageRenderingQueueGTE(size_t length) const
{
    return m_imageRenderingQueue.size() >= length;
}

std::pair<bool, OctData> SignalModel::frontImageRenderingQueue()
{
    QMutexLocker guard(&m_imageRenderingMutex);
    std::pair<bool, OctData> retVal{false, OctData()};
    while(!m_imageRenderingQueue.empty()){
        retVal.second = m_imageRenderingQueue.front();
        retVal.first = true;
        m_imageRenderingQueue.pop();
    }
    return retVal;
}

void SignalModel::freeOctData()
{
    QMutexLocker guard(&m_imageRenderingMutex);
    for(auto it = m_octData.begin(); it != m_octData.end(); ++it){
        delete [] it->second.acqData;
        delete [] it->second.dispData;
    }
    m_octData.clear();
}

OctData SignalModel::handleSimulationSettings(OctData &od)
{
    const auto& settings = userSettings::Instance();
    const bool isSimulation = settings.getIsSimulation();
    const bool isRecording = settings.getIsRecording();
    const bool isSequencial = settings.getIsSequencial();
    const int  endFrame = settings.getEndFrame();
    const int  startFrame = settings.getStartFrame();

    if(isSimulation){
        if(isRecording){
            if(isSequencial){
                od.frameCount = m_simulationFrameCount++;
            }
            if(m_simulationFrameCount <= endFrame){
                saveOct(od);
            }
        } else {
            OCTFile::OctData_t* axsunData = getOctData(0);
            if(m_simulationFrameCount > endFrame){
                m_simulationFrameCount = startFrame;
            }
            od.frameCount = m_simulationFrameCount++;
            od.acqData = axsunData->acqData;
//            LOG1(od.acqData)
            retrieveOct(od);
        }
    }

    return od;
}

OCTFile::OctData_t* SignalModel::getOctData(int index)
{
    QMutexLocker guard(&m_imageRenderingMutex);

    OCTFile::OctData_t* octData{nullptr};
    auto it = m_octData.find(index);

    if(it != m_octData.end())
    {
        octData = &(it->second);
    } else {
        octData = &(m_octData.begin()->second);
    }
//    LOG3(index, octData.acqData, m_octData.size())
    return octData;
}
