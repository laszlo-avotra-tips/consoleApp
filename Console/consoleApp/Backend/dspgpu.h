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

#include <CL/opencl.h>
#include "dsp.h"
#include "buildflags.h"


/*
 * Uses the GPU for DSP calculations
 */
class DSPGPU : public DSP
{
    Q_OBJECT

public:
    ~DSPGPU();

    void init(unsigned int inputLength,
               unsigned int frameLines,
               int inBytesPerRecord,
               int inBytesPerBuffer);

    bool processData(int);
    bool loadFftOutMemoryObjects();

public slots:
    void setAveraging( bool enable ) { doAveraging = enable; }
    void setFrameAverageWeights( int inPrevFrameWeight_percent, int inCurrFrameWeight_percent )
    {
        prevFrameWeight_percent = inPrevFrameWeight_percent / 100.0f;
        currFrameWeight_percent = inCurrFrameWeight_percent / 100.0f;
    }
    void setDisplayAngle( float angle ) { displayAngle_deg = angle; }

private:
    bool  doAveraging; // Instruct the post-process kernel to average two frames at a time.
    float displayAngle_deg;

    // OpenCL state
    cl_device_id     cl_ComputeDeviceId;
    cl_context       cl_Context;

    cl_kernel        cl_PostProcKernel;
    cl_kernel        cl_BandCKernel;
    cl_kernel        cl_WarpKernel;

    cl_program       cl_PostProcProgram;
    cl_program       cl_BandCProgram;
    cl_program       cl_WarpProgram;

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

    quint16 *workingBuffer{nullptr};

    float prevFrameWeight_percent;
    float currFrameWeight_percent;

    float *imData;
    size_t           imDataSize;

    float *reData;
    size_t           reDataSize;

    // OpenCL support
    bool createCLMemObjects( cl_context context );
    bool transformData( unsigned char *data , unsigned char *videoData );

    char       *loadCLProgramSourceFromFile( QString );
    QByteArray  loadCLProgramBinaryFromFile( QString );
    bool buildOpenCLKernel( QString clSourceFile, const char *kernelName, cl_program *program, cl_kernel *kernel );
    bool initOpenCL();
    QString clCreateBufferErrorVerbose(int clError) const;
    bool isClReturnValueSuccess(cl_int ret, int line) const;
};

#endif // DSPGPU_H_
