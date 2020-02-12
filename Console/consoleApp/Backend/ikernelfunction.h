#ifndef IKERNELFUNCTION_H
#define IKERNELFUNCTION_H

#include <CL/opencl.h>

class SignalModel;

class IKernelFunction
{
public:
    virtual ~IKernelFunction() = default;
    virtual bool enqueueInputGpuMemory(cl_command_queue) = 0;
    virtual bool enqueueCallKernelFunction(cl_command_queue) = 0;
    virtual bool initContext(cl_context) = 0;
    virtual void setKernel(cl_kernel kernel) = 0;
    virtual void setSignalModel(SignalModel& signalModel) = 0;
};

#endif // IKERNELFUNCTION_H
