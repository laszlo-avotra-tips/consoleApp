#ifndef SIGNALPROCESSINGFACTORY_H
#define SIGNALPROCESSINGFACTORY_H

#include <CL/opencl.h>
#include <QString>
#include <QByteArray>

#include <map>
#include <memory>


class IKernelFunction;

using OpenClFunction_type = std::pair<cl_program, cl_kernel>;
using OpenClFileMap_type = std::map<QString, QString>; // <kernel function, file name>
using OpenClFunctionMap_type = std::map<QString, OpenClFunction_type >; // <kerlel function < program memory, kerlen memory> >

class SignalProcessingFactory
{
public:
    static SignalProcessingFactory* instance();

    IKernelFunction* getPostFft();

    cl_context getContext() const;

    cl_device_id getComputeDeviceId() const;

    cl_command_queue getCommandQueue() const;

    bool buildKernelFuncionCode(const QString& kernelFunctionName);

    OpenClFunctionMap_type getOpenClFunctionMap() const;

private:
    static SignalProcessingFactory* m_instance;

    SignalProcessingFactory();

    void initOpenClFileMap();
    cl_platform_id getPlatformId() const;
    bool getGpuDeviceInfo(cl_platform_id id, bool isLogging = false);
    bool isClReturnValueSuccess(cl_int ret, int line) const;
    bool buildOpenCLKernel( QString clSourceFile, const char *kernelName, cl_program *program, cl_kernel *kernel );
    char* loadCLProgramSourceFromFile( QString fn);
    QByteArray  loadCLProgramBinaryFromFile( QString );

private:
    OpenClFunctionMap_type m_openClFunctionMap
    {
        {"postfft_kernel",{nullptr,nullptr}},
        {"bandc_kernel",{nullptr,nullptr}},
        {"warp_kernel",{nullptr,nullptr}},
    };

    OpenClFileMap_type m_openClFileMap
    {
        {"postfft_kernel",""},
        {"bandc_kernel",""},
        {"warp_kernel",""},
    };

    cl_platform_id m_platformId{nullptr};
    cl_device_id   m_computeDeviceId{nullptr};
    cl_context     m_context{nullptr};
    cl_command_queue m_commandQueue{nullptr};
    size_t         m_cl_max_workgroup_size{0};
    bool           m_openClSuccess{false};

    std::shared_ptr<IKernelFunction> m_postFftKernelFunction{nullptr};

};

#endif // SIGNALPROCESSINGFACTORY_H
