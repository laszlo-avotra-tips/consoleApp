#include "kernelfunctionbase.h"
#include "signalprocessingfactory.h"


cl_command_queue KernelFunctionBase::m_openClCommandQueue{nullptr};

KernelFunctionBase::KernelFunctionBase(cl_context context)
{
    if(context && !m_baseContext){
        m_baseContext = context;
    }
    if(!m_signalModel){
        m_signalModel = SignalModel::instance();
    }
    if(!m_openClCommandQueue){
        m_openClCommandQueue = SignalProcessingFactory::instance()->getCommandQueue();
    }
}

KernelFunctionBase::~KernelFunctionBase()
{

}

bool KernelFunctionBase::enqueueInputBuffers(const float*, const float*)
{
    return false;
}
