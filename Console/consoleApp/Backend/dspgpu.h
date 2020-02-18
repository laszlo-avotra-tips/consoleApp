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

#include <CL/opencl.h>

#include "dsp.h"
#include "ikernelfunction.h"

#include <QThread>

/*
 * Uses the GPU for DSP calculations
 */
class DSPGPU : public QThread
{
    Q_OBJECT

public:
    virtual ~DSPGPU();

    void init();

    bool processData(int);
    bool readInputBuffers(const float* i, const float* r);

signals:
    void sendWarning( QString );
    void sendError( QString );

public slots:
    void stop( void );
    void setAveraging( bool enable );
    void setFrameAverageWeights( int inPrevFrameWeight_percent, int inCurrFrameWeight_percent );
    void setDisplayAngle( float angle );

private:
    const size_t complexDataSize{1024};

    std::unique_ptr<quint16[]> workingBuffer{nullptr};
    std::unique_ptr<float[]> imData{nullptr};
    std::unique_ptr<float[]> reData{nullptr};

    IKernelFunction* m_postFft{nullptr};
    IKernelFunction* m_bandc{nullptr};
    IKernelFunction* m_warp{nullptr};

    const size_t origin[ 3 ]{ 0, 0, 0 };
    const size_t region[ 3 ]{ SectorWidth_px, SectorHeight_px, 1 };

    OCTFile::FrameData_t *pData{nullptr};
    DSP m_dsp;

private:
    bool initOpenCL();
    bool transformData( unsigned char *data , unsigned char *videoData );

    bool enqueuePostFftKernelFunction();
    bool enqueueBandcKernelFunction();
    bool enqueueWarpKernelFunction();
};

#endif // DSPGPU_H_
