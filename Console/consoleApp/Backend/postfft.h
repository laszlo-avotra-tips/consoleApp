#ifndef POSTFFT_H
#define POSTFFT_H

#include <kernelfunctionbase.h>
#include <defaults.h>
#include <signalmodel.h>

class PostFft : public KernelFunctionBase
{
public:
    PostFft(cl_context context);
    ~PostFft() override;

    bool enqueueInputBuffers(const float* imag, const float* real) override;
    bool enqueueCallKernelFunction() override;
    void setKernel(cl_kernel kernel) override;

private:
    bool initContext();
    bool setKernelArguments(cl_kernel kernel) ;
    void displayFailureMessage(const char* msg, bool isMajor) const;
    bool createFftBuffers();
    bool createLastFrameBuffer();
    bool createImageBuffer();
    cl_mem *getImageBuffer();

private:
    const size_t m_fftMemSize;
    const size_t m_fftGlobalWorkSize[2];

    cl_kernel  m_kernel{nullptr};

    cl_mem m_fftRealBuffer{nullptr}; //0 fftRealOutputMemObj
    cl_mem m_fftImagBuffer{nullptr}; //1 fftImaginaryOutputMemObj
    cl_mem m_lastFrameBuffer{nullptr}; //2 lastFramePreScalingMemObj
    cl_mem m_fftImageBuffer{nullptr}; //3 inputImageMemObj
};

#endif // POSTFFT_H
