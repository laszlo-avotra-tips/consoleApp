#include "signalprocessingfactory.h"

SignalProcessingFactory* SignalProcessingFactory::m_instance{nullptr};

SignalProcessingFactory *SignalProcessingFactory::instance()
{
    if(!m_instance){
        m_instance = new SignalProcessingFactory();
    }
    return m_instance;
}

void SignalProcessingFactory::setContext(cl_context context)
{
    if(!instance()->m_context){
        instance()->m_context = context;
    }
}

SignalProcessingFactory::SignalProcessingFactory()
{

}
