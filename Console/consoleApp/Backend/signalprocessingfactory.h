#ifndef SIGNALPROCESSINGFACTORY_H
#define SIGNALPROCESSINGFACTORY_H

#include <CL/opencl.h>

class IKernelFunction;

class SignalProcessingFactory
{
public:
    static SignalProcessingFactory* instance();
    static void setContext(cl_context context);

    IKernelFunction* getPostFft();

private:
    static SignalProcessingFactory* m_instance;
    cl_context m_context{nullptr};

    SignalProcessingFactory();
};

#endif // SIGNALPROCESSINGFACTORY_H
