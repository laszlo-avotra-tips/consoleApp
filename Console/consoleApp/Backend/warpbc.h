#ifndef WARPBC_H
#define WARPBC_H

#include <kernelfunctionbase.h>
#include <defaults.h>
#include <signalmodel.h>

class WarpBc : public KernelFunctionBase
{
public:
    WarpBc(cl_context context);
    ~WarpBc() override;

    bool enqueueCallKernelFunction() override;
    void setKernel(cl_kernel kernel) override;
private:
    bool initContext();
    bool setKernelArguments(cl_kernel kernel) ;
    bool createWarpBcImageBuffers(cl_context context);

private:
    const size_t m_warpGlobalWorkSize[2];

    cl_kernel  m_kernel{nullptr};
    cl_mem     m_warpImageBuffer{nullptr}; //1 outputImageMemObj;
    cl_mem     m_warpVideoBuffer{nullptr}; //2 outputVideoImageMemObj;

    cl_mem m_beAndCeImage{nullptr}; //1 warpInputImageMemObj
    cl_float m_blackLevel{1.0f}; // 2 blackLevel
    cl_float m_whiteLevel{0.0f}; // 3 whiteLevel
    const size_t m_beAndCeGlobalWorkSize[2];

};

#endif // WARPBC_H
