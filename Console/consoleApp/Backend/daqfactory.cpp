#include "daqfactory.h"
#include "deviceSettings.h"
#include "daqSettings.h"
#include "idaqproxy.h"
#include "filedaq.h"

daqfactory* daqfactory::factory(nullptr);

daqfactory::daqfactory():idaq(nullptr),proxy(nullptr)
{

}

daqfactory *daqfactory::instance()
{
    if(!factory){
        factory = new daqfactory();
    }
    return factory;
}

IDAQ *daqfactory::getdaq()
{
    if( !idaq )
    {
        deviceSettings &setting = deviceSettings::Instance();

        if( setting.current()->isHighSpeed() )
        {
            idaq = new FileDaq();
        }
    }

    return idaq;
}

IDAQ *daqfactory::getProxy()
{
    if(!proxy){
        auto temp = new IDaqProxy();
        temp->setIDaq(getdaq());
        proxy = temp;
    }
    return proxy;
}

