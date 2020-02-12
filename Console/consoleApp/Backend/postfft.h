#ifndef POSTFFT_H
#define POSTFFT_H

#include <ikernelfunction.h>
#include <defaults.h>
#include <signalmodel.h>

class PostFft : public IKernelFunction
{
public:
    PostFft(cl_context context);
    ~PostFft() override;

    bool enqueueInputGpuMemory(cl_command_queue) override;
    bool enqueueCallKernelFunction(cl_command_queue) override;
    bool initContext(cl_context) override;
    void setSignalModel(SignalModel& signalModel) override;
    void setKernel(cl_kernel kernel) override;

private:
    bool setKernelParameters(cl_kernel kernel) ;
    void displayFailureMessage(const char* msg, bool isMajor) const;
    bool createFftBuffers(cl_context context);
    bool createLastFrameBuffer(cl_context context);
    bool createImageBuffer(cl_context context);
    cl_mem *getImageBuffer();

private:
    cl_kernel  m_kernel{nullptr};
    cl_program m_program{nullptr};

    cl_mem m_fftRealBuffer{nullptr}; //0 fftRealOutputMemObj
    cl_mem m_fftImagBuffer{nullptr}; //1 fftImaginaryOutputMemObj
    cl_mem m_lastFrameBuffer{nullptr}; //2 lastFramePreScalingMemObj
    cl_mem m_image{nullptr}; //3 inputImageMemObj

    SignalModel* m_signalModel{nullptr};
};

#endif // POSTFFT_H
