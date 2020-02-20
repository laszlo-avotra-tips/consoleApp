#ifndef SIGNALMODEL_H
#define SIGNALMODEL_H

#include <QObject>
#include <CL/opencl.h>
#include "defaults.h"
#include "octFile.h"
#include <queue>

using OctData = OCTFile::OctData_t;

class SignalModel : public QObject
{
    Q_OBJECT

public:
    static SignalModel* instance();

    OCTFile::OctData_t * getOctData(int index);
    void freeOctData();

    void pushImageRenderingQueue(OctData od);
    void popImageRenderingQueue();
    bool isImageRenderingQueueGTE(size_t length) const;
    std::pair<bool, OctData>  frontImageRenderingQueue();

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

    cl_mem* fftImageBuffer();
    void setFftImageBuffer(const cl_mem &fftImageBuffer);

    cl_float* blackLevel();

    cl_float* whiteLevel();

public slots:
    void setBlackLevel(int blackLevel);
    void setWhiteLevel(int whiteLevel);

public: //data
    const size_t m_oclLocalWorkSize[2]{16,16};
    const cl_uint m_oclWorkDimension{2};
    const size_t* m_oclGlobalWorkOffset{nullptr};
    const cl_uint m_numEventsInWaitlist{0};

public: //functions
    cl_mem* getBeAndCeImageBuffer();
    void setPostBandcImageBuffer(const cl_mem &value);

    cl_float* getCatheterRadiusUm();
    void setCatheterRadiusUm(const cl_float &catheterRadiusUm);

    cl_float* getInternalImagingMaskPx();
    void setInternalImagingMaskPx(const cl_float &internalImagingMaskPx);

    cl_float* getStandardDehthMm();
    void setStandardDehthMm(const cl_float &standardDehthMm);

    cl_int* getStandardDehthS();
    void setStandardDehthS(const cl_int &standardDehthS);

    cl_float* getDisplayAngle();
    void setDisplayAngle(const cl_float &displayAngle);

    cl_int* getReverseDirection();
    void setReverseDirection(const cl_int &reverseDirection);

    cl_float* getFractionOfCanvas();
    void setFractionOfCanvas(const cl_float &fractionOfCanvas);

    cl_int* getImagingDepth();
    void setImagingDepth(const cl_int &imagingDepth);

    cl_mem getWarpImageBuffer();
    void setWarpImageBuffer(const cl_mem &warpImageBuffer);

    cl_mem getWarpVideoBuffer();
    void setWarpVideoBuffer(const cl_mem &warpVideoBuffwr);

private: //functions
    SignalModel();
    void allocateOctData();

private: //data
    static SignalModel* m_instance;

    std::vector<OctData> m_octData;
    QMutex m_imageRenderingMutex;
    std::queue<OctData> m_imageRenderingQueue;

    cl_uint m_linesPerRevolution{592};
    //post fft
    const cl_uint m_iputLength{2048};//4 RescalingDataLength
    const cl_float m_scaleFactor{20000.0f * 255.0f / 65535.0f};//5 scaleFactor
    const cl_uint m_dcNoiseLevel{150};//6 XXX: Empirically measured
    cl_int m_averageVal{0};//7 averageVal // Instruct the post-process kernel to average two frames at a time.
    cl_float m_prevFrameWeight_percent{1.0f - DefaultCurrFrameWeight_Percent / 100.0f}; //8 prevFrameWeight_percent
    cl_float m_currFrameWeight_percent{DefaultCurrFrameWeight_Percent / 100.0f}; //9 currFrameWeight_percent
    cl_int m_isInvertColors{0}; //10 invertColors

    //from post fft to B and C
    cl_mem m_fftImageBuffer{nullptr};

    cl_float m_blackLevel{10.0f}; //2 blackLevel
    cl_float m_whiteLevel{250.0f}; //3 whiteLevel

    //from B and C to warp
    cl_mem m_bAndCimageBuffer{nullptr};

    //warp
    cl_float m_catheterRadiusUm{0.0f}; //3 catheterRadius_um
    cl_float m_internalImagingMaskPx{0.0f}; //4 internalImagingMask_px
    cl_float m_standardDepthMm{0.0f}; //5 standardDepth_mm
    cl_int m_standardDehthS{0}; //6 standardDepth_S
    cl_float m_displayAngle{0.0f}; //7 displayAngle_deg
    cl_int m_reverseDirection{0}; //8 reverseDirection
    cl_float m_fractionOfCanvas{0.0f}; //11 fractionOfCanvas
    cl_int m_imagingDepth; //12 imagingDepth_S

    //post warp
    cl_mem m_warpImageBuffer{nullptr};
    cl_mem m_warpVideoBuffer{nullptr};
};

#endif // SIGNALMODEL_H
