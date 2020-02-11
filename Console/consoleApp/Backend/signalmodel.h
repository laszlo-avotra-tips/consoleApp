#ifndef SIGNALMODEL_H
#define SIGNALMODEL_H

#include <QObject>
#include <CL/opencl.h>

class SignalModel
{
public:
    static SignalModel* instance();

    const cl_uint* iputLength() const;

    const cl_float* scaleFactor() const;

     cl_uint linesPerRevolution() const;
    void setLinesPerRevolution(const cl_uint &linesPerRevolution);

    const cl_uint* dcNoiseLevel() const;

    const cl_int* averageVal() const;
    void setAverageVal(const cl_int &averageVal);

    const cl_float* prevFrameWeight_percent() const;
    void setPrevFrameWeight_percent(const cl_float &prevFrameWeight_percent);

    const cl_float* currFrameWeight_percent() const;
    void setCurrFrameWeight_percent(const cl_float &currFrameWeight_percent);

    const cl_int* isInvertColors() const;
    void setIsInvertColors(const cl_int &isInvertColors);

    cl_mem* postFftImageBuffer();
    void setPostFftImageBuffer(const cl_mem &postFftImageBuffer);

private:
    SignalModel();
    static SignalModel* m_instance;

private:
    cl_uint m_linesPerRevolution{592};
    //post fft
    const cl_uint m_iputLength{2048};//4 RescalingDataLength
    const cl_float m_scaleFactor{20000.0f * 255.0f / 65535.0f};//5 scaleFactor
    const cl_uint m_dcNoiseLevel{150};//6 XXX: Empirically measured
    cl_int m_averageVal{0};//7 averageVal
    cl_float m_prevFrameWeight_percent{.5f}; //8 prevFrameWeight_percent
    cl_float m_currFrameWeight_percent{.5f}; //9 currFrameWeight_percent
    cl_int m_isInvertColors{0}; //10 invertColors

    //from post fft to bandc
    cl_mem m_postFftImageBuffer{nullptr};

};

#endif // SIGNALMODEL_H
