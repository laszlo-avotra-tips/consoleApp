/*
 * dspgpu.cpp
 *
 * Handle the fft signal data from the DAQ
 *    - Post process
 *    - Transform to polar
 *    - Scale output for display
 *
 * The input data is expected to be in two arrays of float, the first
 *
 * Author(s): Chris White, Dennis W. Jackson, Ryan Radjabi modified by lcv
 *
 * Copyright (c) 2012-2018 Avinger, Inc.
 *
 */
#include <QDebug>

#include "defaults.h"
#include "dspgpu.h"
#include "logger.h"
#include "profiler.h"
#include "util.h"
#include "buildflags.h"
#include "deviceSettings.h"
#include "depthsetting.h"
#include "theglobals.h"
#include "signalmanager.h"
#include "playbackmanager.h"
#include "signalmodel.h"
#include "signalprocessingfactory.h"
#include "daqSettings.h"


/*
 * destructor
 *
 * The DSP base constructor is called implicitly
 */
DSPGPU::~DSPGPU()
{
    LOG( INFO, "DSP GPU shutdown" )
    LOG0

    stop();
    wait( 100 ); //ms
}

void DSPGPU::run()
{
    m_dspIsRunning = true;
    int count{0};
    while(m_dspIsRunning){
        msleep(1);
        if(++count % 3000 == 0){
            auto now = QTime::currentTime().toString("HH:mm:ss:zz");
//            LOG2(count, now)
        }
    }
}

/*
 * init
 *
 * Initialize memory and the GPU hardware
 */
void DSPGPU::init( )
{
    // call the common initilization steps
    m_dsp.init();

    auto &settings = deviceSettings::Instance();
    auto smi = SignalModel::instance();

    float catheterRadius_um = float(settings.current()->getCatheterRadius_um());
    smi->setCatheterRadiusUm(catheterRadius_um);

    float internalImagingMask_px = float(settings.current()->getInternalImagingMask_px());
    smi->setInternalImagingMaskPx(internalImagingMask_px);

    // Contrast stretch defaults (no stretch)
    float blackLevel = BrightnessLevels_HighSpeed.defaultValue;
    float whiteLevel = ContrastLevels_HighSpeed.defaultValue;
    smi->setBlackLevel(int(blackLevel));
    smi->setWhiteLevel(int(whiteLevel));


    const size_t inSamplesPerBuffer = size_t(settings.current()->getLinesPerRevolution()) * DaqSettings::Instance().getRecordLength();
    LOG1(inSamplesPerBuffer)

    // XXX Need to pass these into the DSP so we dont copy buffers there
    // for opencl
    try {
        workingBuffer = std::make_unique<quint16 []>(inSamplesPerBuffer);
        imData = std::make_unique<float []>(complexDataSize);
        reData = std::make_unique<float []>(complexDataSize);
    } catch (std::bad_alloc e) {
        emit sendError( e.what() );
    }

    // Configure the DSP
    if( !initOpenCL() )
    {
        displayFailureMessage( tr( "Could not initialize OpenCL." ), true );
    }

}

bool DSPGPU::processData(int index)
{
    bool success(false);

    pOctData = TheGlobals::instance()->getOctData(index);
    if(pOctData){
        if( !transformData( pOctData->dispData, pOctData->videoData ) )   //Success if return true
        {
            LOG( WARNING, "Failed to transform data on GPU." )
        }

        pOctData->encoderPosition = 0; // NOT USED for fast-OCT
        pOctData->timeStamp       = m_dsp.getTimeStamp();
        pOctData->milliseconds    = quint16(m_dsp.getMilliseconds());

        success = true;

        TheGlobals::instance()->pushImageRenderingQueue(*pOctData);
    } else{
        LOG1(pOctData)
    }
    return success;
}

bool DSPGPU::enqueuePostFftKernelFunction()
{
    bool success{false};
    if(m_postFft){
        success = m_postFft->enqueueCallKernelFunction();
    }
    return success;
}

bool DSPGPU::enqueueBandcKernelFunction()
{
    bool success{false};
    if(m_bandc){
        success = m_bandc->enqueueCallKernelFunction();
    }
    return success;
}

bool DSPGPU::enqueueWarpKernelFunction()
{
    bool success{false};
    if(m_warp){
        success = m_warp->enqueueCallKernelFunction();
    }
    return success;
}

/*
 * initOpenCL
 *
 * Enumerate and initialize OpenCL devices, programs and kernels. This
 * function is very long and only gets worse with more kernels. Think
 * about how to break it up. Maybe this function is table driven and
 * loads all kernels from a structure?
 */
bool DSPGPU::initOpenCL()
{
    qDebug() << "initOpenCL start";

    auto dev = deviceSettings::Instance().current();
    auto smi = SignalModel::instance();

    auto standardDepthMm = dev->getImagingDepthNormal_mm();
    smi->setStandardDehthMm(standardDepthMm);

    auto standardDepthS = dev->getALineLengthNormal_px();
    smi->setStandardDehthS(standardDepthS);

    auto spf = SignalProcessingFactory::instance();
    m_postFft = spf->getPostFft();
    m_bandc = spf->getBandC();
    m_warp = spf->getWarp();

    spf->buildKernelFunctions();

    qDebug() << "DSPGPU: OpenCL init complete.";

    return true;
}

/*
 * transformData
 *
 * post process the FFT input to 8-bit and warp to sector.
 */
bool DSPGPU::transformData( unsigned char *dispData, unsigned char *videoData )
{
    int            clStatus;

    if(!enqueuePostFftKernelFunction()){
        return false;
    }

    /*
     * Adjust brightness and contrast
     */
    if(!enqueueBandcKernelFunction()){
        return false;
    }

    if(!enqueueWarpKernelFunction()){
        return false;
    }

    auto spf = SignalProcessingFactory::instance();

    // Do all the work that was queued up on the GPU
    clStatus = clFinish( spf->getCommandQueue() );
    if( clStatus != CL_SUCCESS )
    {
        qDebug() << "DSP: Failed to issue and complete all queued commands: " << clStatus;
        return false;
    }


   auto smi = SignalModel::instance();
   /*
    * read out the display frame
    */
   clStatus = clEnqueueReadImage(  spf->getCommandQueue(), smi->getWarpImageBuffer(), CL_TRUE, origin, region, 0, 0, dispData, 0, nullptr, nullptr );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to read back final image data from warp kernel: " << clStatus;
       return false;
   }

   /*
    * read out the video frame
    */
   clStatus = clEnqueueReadImage(  spf->getCommandQueue(), smi->getWarpVideoBuffer(), CL_TRUE, origin, region, 0, 0, videoData, 0, nullptr, nullptr );
   if( clStatus != CL_SUCCESS )
   {
       qDebug() << "DSP: Failed to read back video image data from warp kernel: " << clStatus;
       return false;
   }

   return true;
}


bool DSPGPU::readInputBuffers(const float *imag, const float *real)
{
    if(m_postFft && imag && real){
        m_postFft->enqueueInputBuffers( imag, real);
        return true;
    }
    return false;
}

void DSPGPU::stop()
{
    m_dspIsRunning = false;
}

void DSPGPU::setAveraging(bool enable)
{
    SignalModel::instance()->setAverageVal(enable);
}

void DSPGPU::setFrameAverageWeights(int inPrevFrameWeight_percent, int inCurrFrameWeight_percent)
{
    SignalModel::instance()->setPrevFrameWeight_percent(inPrevFrameWeight_percent / 100.0f);
    SignalModel::instance()->setCurrFrameWeight_percent(inCurrFrameWeight_percent / 100.0f);
}

void DSPGPU::setDisplayAngle(float angle)
{
    SignalModel::instance()->setDisplayAngle(angle);
}
