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
//#include "clAmdFft.h"
#include "dsp.h"
#include "buildflags.h"

#ifdef UNIT_TEST
#define private public
#endif

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
               unsigned int inBytesPerRecord,
               unsigned int inBytesPerBuffer,
               int inChannelCount);


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

    cl_kernel        cl_RescaleKernel;
    cl_kernel        cl_PostProcKernel;
    cl_kernel        cl_BandCKernel;
    cl_kernel        cl_WarpKernel;

    cl_program       cl_RescaleProgram;
    cl_program       cl_PostProcProgram;
    cl_program       cl_BandCProgram;
    cl_program       cl_WarpProgram;

    cl_command_queue cl_Commands;

//    //    FFT State
//    clAmdFftPlanHandle hCl_fft_plan;

    size_t           cl_max_workgroup_size;

    float           *windowBuffer;
    cl_mem           rescaleInputMemObj;
    size_t           rescaleInputMemObjSize;

    cl_mem           rescaleOutputMemObj;
    size_t           rescaleOutputMemObjSize;

    cl_mem           windowMemObj;
    size_t           windowMemObjSize;

    cl_mem           fftImaginaryInputMemObj;
    size_t           fftImaginaryInputMemObjSize;

//    cl_mem           postProcOutputMemObj;
//    size_t           postProcOutputMemObjSize;

    float           *fftImaginaryBuffer;
    size_t           fftImaginaryBufferSize;

    cl_mem           fftRealOutputMemObj;
    size_t           fftRealOutputMemObjSize;

    cl_mem           fftImaginaryOutputMemObj;
    size_t           fftImaginaryOutputMemObjSize;

    cl_mem           rescaleWholeSamplesMemObj;
    size_t           rescaleWholeSamplesMemObjSize;

    cl_mem           rescaleFracSamplesMemObj;
    size_t           rescaleFracSamplesMemObjSize;

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

    quint16 *workingBuffer[ 2 ];
    unsigned char *pPostProcOutputFrame;
    size_t           pPostProcOutputFrameSize;

    float prevFrameWeight_percent;
    float currFrameWeight_percent;

    float *imData;
    size_t           imDataSize;

    float *reData;
    size_t           reDataSize;

    // OpenCL support
    bool createCLMemObjects( cl_context context );

    unsigned int rescale( const unsigned short *inputData );
    void processData( void );
    bool transformData( unsigned char *data , unsigned char *videoData, int index = 0 );

    char       *loadCLProgramSourceFromFile( QString );
    QByteArray  loadCLProgramBinaryFromFile( QString );
    bool buildOpenCLKernel( QString clSourceFile, const char *kernelName, cl_program *program, cl_kernel *kernel );
    bool initOpenCL();
    void computeFFTWindow();
    bool initOpenCLFFT();
    QString clCreateBufferErrorVerbose(int clError) const;
    bool computeTheFFT(cl_mem rescaleOut, cl_mem &fftOutReal, cl_mem &fftOutImag); //const quint16 *
    bool computeTheFFT(const quint16 * pDataIn, cl_mem &fftOutReal, cl_mem &fftOutImag);
    bool isClReturnValueSuccess(cl_int ret, int line) const;
};

#endif // DSPGPU_H_
