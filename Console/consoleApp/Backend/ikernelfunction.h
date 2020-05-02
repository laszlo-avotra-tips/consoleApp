#ifndef IKERNELFUNCTION_H
#define IKERNELFUNCTION_H

#include <CL/opencl.h>

class IKernelFunction
{
public:
    virtual ~IKernelFunction() = default;
    virtual bool enqueueInputBuffers(const float*, const float*) = 0;
    virtual bool enqueueCallKernelFunction() = 0;
    virtual void setKernel(cl_kernel kernel) = 0;
};

#endif // IKERNELFUNCTION_H
