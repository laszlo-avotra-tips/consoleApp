/*
 * dspgpu.h
 *
 * Interface for the GPU-based digital signal processing functions.
 *
 * Author: Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#ifndef DSPGPU_H_
#define DSPGPU_H_

#include <memory>
#include <map>
#include <vector>

#include <CL/opencl.h>

#include "dsp.h"
#include "buildflags.h"
#include "kernelfunctionlogarithmicpowerdensity.h" //lpd

using OpenClFunction_type = std::pair<cl_program,cl_kernel>;
using OpenClFileMap_type = std::map<QString,QString>; // <kernel function, file name>
using OpenClFunctionMap_type = std::map<QString, OpenClFunction_type >; // <kerlel function < program memory, kerlen memory> >
/*
 * Uses the GPU for DSP calculations
 */
class DSPGPU : public DSP
{
    Q_OBJECT

public:
    ~DSPGPU();

    void init(size_t inputLength,
              size_t frameLines,
              size_t inBytesPerRecord,
              size_t inBytesPerBuffer);

    bool processData(int);
    bool loadFftOutMemoryObjects();

public slots:
    void setAveraging( bool enable );
    void setFrameAverageWeights( int inPrevFrameWeight_percent, int inCurrFrameWeight_percent );
    void setDisplayAngle( float angle );

private:
    bool  doAveraging; // Instruct the post-process kernel to average two frames at a time.
    float displayAngle_deg;

    // OpenCL state
    cl_device_id     cl_ComputeDeviceId;
    cl_context       cl_Context;

    OpenClFunctionMap_type m_openClFunctionMap
    {
        {"postproc_kernel",{nullptr,nullptr}},
        {"bandc_kernel",{nullptr,nullptr}},
        {"warp_kernel",{nullptr,nullptr}},
    };

    OpenClFileMap_type m_openClFileMap
    {
        {"postproc_kernel",""},
        {"bandc_kernel",""},
        {"warp_kernel",""},
    };


    cl_command_queue cl_Commands;

    size_t           cl_max_workgroup_size;

    cl_mem           fftRealOutputMemObj;
    size_t           fftRealOutputMemObjSize;

    cl_mem           fftImaginaryOutputMemObj;
    size_t           fftImaginaryOutputMemObjSize;

    cl_mem           lastFramePreScalingMemObj; // The last frame. Used for frame-averaging
    size_t           lastFramePreScalingMemObjSize;

    cl_mem           inputImageMemObj;
    size_t           inputImageMemObjSize;

    cl_mem           warpInputImageMemObj;
    size_t           warpInputImageMemObjSize;

    cl_mem           outputImageMemObj;
    size_t           outputImageMemObjSize;

    cl_mem           outputVideoImageMemObj;
    size_t           outputVideoImageMemObjSize;

    std::unique_ptr<quint16[]> workingBuffer{nullptr};
    std::unique_ptr<float[]> imData{nullptr};
    std::unique_ptr<float[]> reData{nullptr};

    float prevFrameWeight_percent;
    float currFrameWeight_percent;

    const size_t complexDataSize{1024};
    const size_t oclLocalWorkSize[2]{16,16};
    const cl_uint oclWorkDimension{2};
    const size_t* oclGlobalWorkOffset{nullptr};
    const cl_uint numEventsInWaitlist{0};

private:
    KernelFunctionLogarithmicPowerDensity m_lpd;

private:
    // OpenCL support
    bool createCLMemObjects( cl_context context );
    bool transformData( unsigned char *data , unsigned char *videoData );

    char       *loadCLProgramSourceFromFile( QString );
    QByteArray  loadCLProgramBinaryFromFile( QString );
    bool buildOpenCLKernel( QString clSourceFile, const char *kernelName, cl_program *program, cl_kernel *kernel );
    bool initOpenCL();
    QString clCreateBufferErrorVerbose(int clError) const;
    bool isClReturnValueSuccess(cl_int ret, int line) const;
    void initOpenClFileMap();
    cl_platform_id getPlatformId() const;
    bool getGpuDeviceInfo(cl_platform_id id, bool isLogging = false);

    bool callPostProcessKernel() const;
    bool callBandcKernel() const;
    bool callWarpKernel() const;
};

#endif // DSPGPU_H_
