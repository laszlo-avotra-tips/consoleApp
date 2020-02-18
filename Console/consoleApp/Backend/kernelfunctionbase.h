#ifndef KERNELFUNCTIONBASE_H
#define KERNELFUNCTIONBASE_H

#include <ikernelfunction.h>
#include <signalmodel.h>
#include <imagedescriptor.h>


class KernelFunctionBase : public IKernelFunction
{
public:
    KernelFunctionBase(cl_context context = nullptr);
    ~KernelFunctionBase() override;
    bool enqueueInputBuffers(const float *, const float*) override;

protected:
    static cl_command_queue m_openClCommandQueue; //there is only one command queue
    ImageDescriptor m_imageDescriptor;

    cl_context m_baseContext{nullptr};
    SignalModel* m_signalModel{nullptr};
};

#endif // KERNELFUNCTIONBASE_H
