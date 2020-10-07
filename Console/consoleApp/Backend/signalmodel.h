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

    void pushImageRenderingQueue(const OctData& od);
    void popImageRenderingQueue();
    bool isImageRenderingQueueGTE(size_t length) const;
    std::pair<bool, OctData>  frontImageRenderingQueue();

    const cl_uint* getInputLength() const;

    const cl_float* scaleFactor() const;

    void storeAdvancedViewFftData(char* data);

     cl_uint linesPerRevolution() const;
    void setLinesPerRevolution(const cl_uint &linesPerRevolution);

    const cl_uint* dcNoiseLevel() const;

    const cl_int* isAveragingNoiseReduction() const;

    const cl_float* prevFrameWeight_percent() const;

    const cl_float* currFrameWeight_percent() const;

    const cl_int* isInvertOctColors() const;

    const cl_mem *fftImageBuffer() const;
    void setFftImageBuffer(const cl_mem &fftImageBuffer);

    const cl_int *blackLevel() const;

    const cl_int *whiteLevel() const;

public slots:
    void setIsAveragingNoiseReduction(bool isAveragingNoiseReduction);
    void setCurrFrameWeight_percent(int currFrameWeight_percent);
    void setIsInvertColors(bool isInvertOctColors);

    void setBlackLevel(int blackLevel);
    void setWhiteLevel(int whiteLevel);

public: //data
    const size_t m_oclLocalWorkSize[2]{16,16};
    const cl_uint m_oclWorkDimension{2};
    const size_t* m_oclGlobalWorkOffset{nullptr};
    const cl_uint m_numEventsInWaitlist{0};

public: //functions
    const cl_mem* getBeAndCeImageBuffer() const;
    void setPostBandcImageBuffer(const cl_mem &value);

//    const cl_float* getCatheterRadius_um() const;
//    void setCatheterRadius_um(const cl_float &catheterRadius_um);

    const cl_float* getInternalImagingMask_px() const;
    void setInternalImagingMask_px(const cl_float &internalImagingMask_px);

    const cl_int* getALineLengthNormal_px() const;
    void setALineLengthNormal_px(const cl_int &aLineLengthNormal_px);

    const cl_float* getDisplayAngle() const;
    void setDisplayAngle(const cl_float &displayAngle);

    const cl_int* getIsDistalToProximalView() const;
    void setIsDistalToProximalView(const cl_int &isDistalToProximalView);

    const cl_float* getFractionOfCanvas() const;
    void setFractionOfCanvas(const cl_float &fractionOfCanvas);

    const cl_int* getImagingDepth_S() const;
    void setImagingDepth_S(const cl_int &imagingDepth_S);

    cl_mem getWarpImageBuffer();
    void setWarpImageBuffer(const cl_mem &warpImageBuffer);

    cl_mem getWarpVideoBuffer();
    void setWarpVideoBuffer(const cl_mem &warpVideoBuffwr);

    const cl_int* getSectorWidth_px() const;
    const cl_int* getSectorHeight_px() const;

    void setAdvacedViewSourceFrameNumber(int frameNumber);

    const uint8_t* getAdvancedViewFrame() const;

    size_t getBufferLength() const;
    void setBufferLength(const size_t &bufferLength);

private: //functions
    SignalModel();
    void allocateOctData();

private: //data
    static SignalModel* m_instance;

    std::vector<OctData> m_octData;
    QMutex m_imageRenderingMutex;
    std::queue<OctData> m_imageRenderingQueue;

    cl_uint m_linesPerRevolution{1184};
    //post fft
    const cl_uint m_inputLength{1024};//4 RescalingDataLength
    const cl_float m_scaleFactor{10000.0f * 255.0f / 65535.0f};//5 scaleFactor
    const cl_uint m_dcNoiseLevel{150};//6 XXX: Empirically measured
    cl_int m_isAveragingNoiseReduction{false};//7 averageVal // Instruct the post-process kernel to average two frames at a time.
    cl_float m_prevFrameWeight_percent{1.0f - DefaultCurrFrameWeight_Percent * 0.01f}; //8 prevFrameWeight_percent
    cl_float m_currFrameWeight_percent{DefaultCurrFrameWeight_Percent * 0.01f}; //9 currFrameWeight_percent
    cl_int m_isInvertOctColors{false}; //10 invertColors

    //from post fft to B and C
    cl_mem m_fftImageBuffer{nullptr};

    cl_int m_blackLevel{0}; //2 blackLevel
    cl_int m_whiteLevel{0}; //3 whiteLevel

    //from B and C to warp
    cl_mem m_bAndCimageBuffer{nullptr};

    //warp
    cl_float m_catheterRadius_um{0.0f}; //3 catheterRadius_um
    cl_float m_internalImagingMask_px{0.0f}; //4 internalImagingMask_px
    cl_int m_aLineLengthNormal_px{0}; //6 standardDepth_S
    cl_float m_displayAngle{0.0f}; //7 displayAngle_deg
    cl_int m_isDistalToProximalView{0}; //8 reverseDirection
    const cl_int& m_sectorWidth_px{SectorWidth_px}; //9
    const cl_int& m_sectorHeight_px{SectorHeight_px}; //10
    cl_float m_fractionOfCanvas{0.0f}; //11 fractionOfCanvas
    cl_int m_imagingDepth_S; //12 imagingDepth_S

    //post warp
    cl_mem m_warpImageBuffer{nullptr};
    cl_mem m_warpVideoBuffer{nullptr};

    int m_dvacedViewSourceFrameNumber{0};
    size_t m_bufferLength{0};
};

#endif // SIGNALMODEL_H
