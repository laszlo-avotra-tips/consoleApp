#ifndef IKERNELFUNCTION_H
#define IKERNELFUNCTION_H

#include <CL/opencl.h>

class IKernelFunction
{
public:
    virtual ~IKernelFunction() = default;
    virtual bool enqueueInputGpuMemory(cl_command_queue) = 0;
    virtual bool enqueueCallKernelFunction(cl_command_queue) = 0;
    virtual bool enqueueOutputGpuMemory(cl_command_queue) = 0;
    virtual bool initContext(cl_context) = 0;
    virtual bool setKernelParameters(cl_kernel kernel) = 0;

public:
    const size_t m_oclLocalWorkSize[2]{16,16};
    const cl_uint m_oclWorkDimension{2};
    const size_t* m_oclGlobalWorkOffset{nullptr};
    const cl_uint m_numEventsInWaitlist{0};

};

#endif // IKERNELFUNCTION_H
