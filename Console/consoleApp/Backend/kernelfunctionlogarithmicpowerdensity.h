#ifndef KERNELFUNCTIONLOGARITHMICPOWERDENSITY_H
#define KERNELFUNCTIONLOGARITHMICPOWERDENSITY_H

#include <ikernelfunction.h>
#include <CL/opencl.h>
#include <defaults.h>

class KernelFunctionLogarithmicPowerDensity : public IKernelFunction
{
public:
    KernelFunctionLogarithmicPowerDensity(cl_context context);
    ~KernelFunctionLogarithmicPowerDensity() override;

    void setContext(cl_context context);
    void setIsAveraging(bool isAveraging);
    void setIsInvertColors(bool isInvertColors);
    void setFftBuffers(cl_mem real, cl_mem imag);
    void setLastFrameBuffer(cl_mem lastFrameBuffer);
    cl_mem getImageBuffer() const;
    cl_mem createImageBuffer();

private:
    cl_context m_context;
    cl_mem m_fftRealBuffer{nullptr}; //0 fftRealOutputMemObj
    cl_mem m_fftImagBuffer{nullptr}; //1 fftImaginaryOutputMemObj
    cl_mem m_lastFrameBuffer{nullptr}; //2 lastFramePreScalingMemObj
    cl_mem m_image{nullptr}; //3 inputImageMemObj
    const cl_uint m_inputLength{2048}; //4 RescalingDataLength
    const cl_float m_scaleFactor{20000.0f * 255.0f / 65535.0f}; //5 scaleFactor
    const cl_uint m_dcNoiseLevel{150}; //6 XXX: Empirically measured
    cl_int m_isAveraging{0}; //7 averageVal
    cl_float m_prevFrameWeight_percent{1.0}; //8 prevFrameWeight_percent
    cl_float m_currFrameWeight_percent{1.0}; //9 currFrameWeight_percent
    cl_int m_isInvertColors{0}; //10 invertColors

};

#endif // KERNELFUNCTIONLOGARITHMICPOWERDENSITY_H
