#include "idaqproxy.h"

IDaqProxy::IDaqProxy():m_idaq(nullptr)
{

}

void IDaqProxy::setIDaq(IDAQ *obj)
{
    m_idaq = obj;
}

void IDaqProxy::stop()
{
    if(m_idaq)
    {
        m_idaq->stop();
    }
}

void IDaqProxy::pause()
{
    if(m_idaq)
    {
        m_idaq->pause();
    }
}

void IDaqProxy::resume()
{
    if(m_idaq)
    {
        m_idaq->resume();
    }
}

void IDaqProxy::init()
{
    if(m_idaq)
    {
        m_idaq->init();
    }
}

QString IDaqProxy::getDaqLevel()
{
    if(m_idaq)
    {
        m_idaq->getDaqLevel();
    }

    return QString();
}

long IDaqProxy::getRecordLength() const
{
    if(m_idaq)
    {
        return m_idaq->getRecordLength();
    }

    return 0;
}

void IDaqProxy::enableAuxTriggerAsTriggerEnable(bool enable)
{
    if(m_idaq)
    {
        m_idaq->enableAuxTriggerAsTriggerEnable(enable);
    }
}

bool IDaqProxy::configure()
{
    return false;
}

bool IDaqProxy::getData()
{
    return false;
}
