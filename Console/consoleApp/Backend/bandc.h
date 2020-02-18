#ifndef BEANDCE_H
#define BEANDCE_H

#include <kernelfunctionbase.h>
#include <defaults.h>
#include <signalmodel.h>

class BeAndCe : public KernelFunctionBase
{
public:
    BeAndCe(cl_context context);
    ~BeAndCe() override;

    bool enqueueCallKernelFunction() override;
    void setKernel(cl_kernel kernel) override;

private:
    bool initContext();
    bool setKernelArguments(cl_kernel kernel) ;
    bool createBeAndCeImageBuffer(cl_context context);

private:
    cl_kernel  m_kernel{nullptr};

    cl_mem m_beAndCeImage{nullptr}; //1 warpInputImageMemObj
    cl_float m_blackLevel{1.0f}; // 2 blackLevel
    cl_float m_whiteLevel{0.0f}; // 3 whiteLevel
};

#endif // BEANDCE_H
