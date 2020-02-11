#include "signalmodel.h"

SignalModel* SignalModel::m_instance{nullptr};

SignalModel::SignalModel()
{
}

cl_mem* SignalModel::postFftImageBuffer()
{
    return &m_postFftImageBuffer;
}

void SignalModel::setPostFftImageBuffer(const cl_mem &postFftImageBuffer)
{
    m_postFftImageBuffer = postFftImageBuffer;
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
