#include "daqfactory.h"
#include "deviceSettings.h"
#include "daqSettings.h"
#include "filedaq.h"

daqfactory* daqfactory::factory(nullptr);

daqfactory::daqfactory():idaq(nullptr)
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
