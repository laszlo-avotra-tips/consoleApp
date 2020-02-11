#ifndef POSTFFT_H
#define POSTFFT_H

#include <ikernelfunction.h>
#include <defaults.h>

class PostFft : public IKernelFunction
{
public:
    PostFft();
    PostFft(cl_context context);
    ~PostFft() override;

    bool enqueueInputGpuMemory(cl_command_queue) override;
    bool enqueueCallKernelFunction(cl_command_queue) override;
    bool enqueueOutputGpuMemory(cl_command_queue) override;
    bool initContext(cl_context) override;
    bool setKernelParameters(cl_kernel kernel) override;

    void setIsAveraging(bool isAveraging);
    void setIsInvertColors(bool isInvertColors);
    cl_mem *getImageBuffer();
    void setKernel(cl_kernel kernel);
    void setPrevFrameWeightPercent(cl_float val);
    void setCurrFrameWeightPercent(cl_float val);

private:
    void displayFailureMessage(const char* msg, bool isMajor) const;
    bool createFftBuffers(cl_context context);
    bool createLastFrameBuffer(cl_context context);
    bool createImageBuffer(cl_context context);

private:
    cl_kernel  m_kernel{nullptr};
    cl_program m_program{nullptr};

    cl_mem m_fftRealBuffer{nullptr}; //0 fftRealOutputMemObj
    cl_mem m_fftImagBuffer{nullptr}; //1 fftImaginaryOutputMemObj
    cl_mem m_lastFrameBuffer{nullptr}; //2 lastFramePreScalingMemObj
    cl_mem m_image{nullptr}; //3 inputImageMemObj

    const cl_uint m_inputLength{2048}; //4 RescalingDataLength
    const cl_uint m_linesPerRevolution{0};
    const cl_float m_scaleFactor{20000.0f * 255.0f / 65535.0f}; //5 scaleFactor
    const cl_uint m_dcNoiseLevel{150}; //6 XXX: Empirically measured
    cl_int m_isAveraging{0}; //7 averageVal

    cl_float m_prevFrameWeight_percent{1.0}; //8 prevFrameWeight_percent
    cl_float m_currFrameWeight_percent{1.0}; //9 currFrameWeight_percent
    cl_int m_isInvertColors{0}; //10 invertColors

};

#endif // POSTFFT_H
