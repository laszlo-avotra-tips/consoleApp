#ifndef WARP_H
#define WARP_H

#include <kernelfunctionbase.h>
#include <defaults.h>
#include <signalmodel.h>

class Warp : public KernelFunctionBase
{
public:
    Warp(cl_context context);
    ~Warp() override;

    bool enqueueCallKernelFunction() override;
    void setKernel(cl_kernel kernel) override;

private:
    bool initContext();
    bool setKernelArguments(cl_kernel kernel) ;
    bool createWarpImageBuffers(cl_context context);

private:
    const size_t m_warpGlobalWorkSize[2];

    cl_kernel  m_kernel{nullptr};
    cl_mem     m_warpImageBuffer{nullptr}; //1 outputImageMemObj;
    cl_mem     m_warpVideoBuffer{nullptr}; //2 outputVideoImageMemObj;
};

#endif // WARP_H
